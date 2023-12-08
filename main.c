#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

// Define boolean values for readability
#define TRUE 1
#define FALSE 0

// Uninitialized pointer definition
#define UNINITIALIZED (NULL)

// Type definitions for clarity
typedef unsigned int HashValue;
typedef void* KeyType;
typedef void* ValueType;
typedef int Boolean;

// Node structure for the hash table
typedef struct _Node {
    struct _Node* markedNext;  // Pointer to the next node, possibly marked
    HashValue hash;            // Hash value of the key
    KeyType key;               // Key stored in the node
    ValueType value;           // Value stored in the node
} Node;

// Concurrent hash table structure
typedef struct {
    Node** buckets;            // Array of pointers to hash table buckets
    unsigned count;            // Number of elements in the hash table
    unsigned size;             // Current size of the buckets array
    Boolean lockEnabled;       // Flag to indicate if locking is enabled
} ConcurrentHashTable;

static unsigned int
reverse_value (unsigned int k)
{
	int i;
	unsigned int r = 0;
	for (i = 0; i < 32; ++i) {
		unsigned int bit = (k & (1 << i)) >> i;
		r |= bit << (31 - i);
	}
	return r;
}

static inline unsigned int hash_key (KeyType key)
{
	return (unsigned int)((uintptr_t)(key) * 2654435761u);
}

static inline unsigned int
hash_regular_key (unsigned int k)
{
	return reverse_value (k | 0x80000000);
}

static inline unsigned int
hash_dummy_key (unsigned int k)
{
	return reverse_value (k & ~0x80000000);
}

static inline Boolean
is_dummy_Node (unsigned int k)
{
	return (k & 0x1) == 0;
}

static inline Boolean
is_regular_Node (unsigned int k)
{
	return (k & 0x1) == 1;
}
// Key for thread-specific data to store hazard pointers
pthread_key_t hazard_pointer_key;



// --- Utility Functions ---

static inline Node* mark_node_ptr(Node *node, uintptr_t mark) {
    return (Node*)(((uintptr_t)node) | mark);
}

static inline Node* get_actual_node(Node *markedNode) {
    return (Node*)(((uintptr_t)markedNode) & ~((uintptr_t)1));
}

static inline uintptr_t get_mark_bit(Node *markedNode) {
    return ((uintptr_t)markedNode) & 1;
}

static void delete_node(Node *node) {
    assert(get_mark_bit(node) == 0);
    free(get_actual_node(node));
}

// --- Hazard Pointers Implementation ---

static void** get_hazard_pointer_table(void) {
    void **hazardTable = pthread_getspecific(hazard_pointer_key);
    if (!hazardTable) {
        hazardTable = calloc(3, sizeof(void*));
        pthread_setspecific(hazard_pointer_key, hazardTable);
    }
    return hazardTable;
}

static void set_hazard_pointer(void **hazardTable, int index, void *ptr) {
    hazardTable[index] = ptr;
}

static void clear_hazard_pointer(void **hazardTable, int index) {
    hazardTable[index] = NULL;
}

static void* get_hazard_pointer(void **hazardTable, int index) {
    return hazardTable[index];
}

static void free_hazard_table(void *hazardTable) {
    free(hazardTable);
}

// Initialization function for hazard pointers
static void init_hazard_pointers() {
    pthread_key_create(&hazard_pointer_key, free_hazard_table);
}





// --- List Operations ---

static Node* list_find_hp(ConcurrentHashTable *hashTable, unsigned bucket, KeyType key, HashValue hash, Node ***outPrev) {
    Node **table = get_hazard_pointer_table();
    Node *cur, *next, **prev;

    table = get_hazard_pointer_table();
    Node **head = &table[bucket];
    prev = head;
    cur = get_actual_node(*prev);

    while (TRUE) {
        if (!cur)
            break;
        next = get_actual_node(cur->markedNext);

        if (*prev != mark_node_ptr(cur, 0))
            break; // Restart if the previous node has changed

        if (!get_mark_bit(cur->markedNext)) {
            if (cur->hash >= hash && (cur->hash > hash || cur->key == key))
                break;

            prev = &cur->markedNext;
        } else {
            if (__sync_bool_compare_and_swap(prev, mark_node_ptr(cur, 0), mark_node_ptr(next, 0)))
                delete_node(cur);
            else
                break; // Restart if CAS failed
        }
        cur = next;
    }

    *outPrev = prev;
    return cur;
}

static Node* list_insert_hp(ConcurrentHashTable *hashTable, unsigned bucket, Node *node) {
    Node *res, **prev;

    while (TRUE) {
        res = list_find_hp(hashTable, bucket, node->key, node->hash, &prev);
        if (res && res->hash == node->hash && res->key == node->key)
            return res; // Node already exists

        node->markedNext = mark_node_ptr(res, 0);
        if (__sync_bool_compare_and_swap(prev, mark_node_ptr(res, 0), mark_node_ptr(node, 0)))
            return node; // Successful insertion
    }
}

static Node* list_delete_hp(ConcurrentHashTable *hashTable, unsigned bucket, KeyType key, HashValue hash) {
    Node *res, **prev, *next;

    while (TRUE) {
        res = list_find_hp(hashTable, bucket, key, hash, &prev);
        if (!res || res->hash != hash || res->key != key)
            return NULL; // Node not found

        next = get_actual_node(res->markedNext);
        if (!__sync_bool_compare_and_swap(&res->markedNext, mark_node_ptr(next, 0), mark_node_ptr(next, 1)))
            continue; // Failed to mark the node, retry

        if (__sync_bool_compare_and_swap(prev, mark_node_ptr(res, 0), mark_node_ptr(next, 0)))
            delete_node(res); // Node deleted successfully

        return res; // Return the deleted node
    }
}



// --- Table Management ---

static unsigned get_parent_bucket(unsigned bucket) {
    int i;
    for (i = 31; i >= 0; --i) {
        if (bucket & (1 << i))
            return bucket & ~(1 << i);
    }
    return 0;
}
static void initialize_bucket(ConcurrentHashTable *hashTable, unsigned bucket) {
    Node *dummyNode = calloc(1, sizeof(Node));
    dummyNode->key = (KeyType)(uintptr_t)bucket;
    dummyNode->hash = hash_dummy_key(bucket);

    Node **buckets = hashTable->buckets;
    if (buckets[bucket] == UNINITIALIZED) {
        Node *parentBucketNode = list_insert_hp(hashTable, get_parent_bucket(bucket), dummyNode);
        if (parentBucketNode != dummyNode) {
            free(dummyNode);
        }
        __sync_bool_compare_and_swap(&buckets[bucket], UNINITIALIZED, dummyNode);
    }
}

static void resize_table(ConcurrentHashTable *hashTable) {
    unsigned oldSize = hashTable->size;
    unsigned newSize = oldSize * 2;
    Node **newBuckets = calloc(newSize, sizeof(Node*));

    memcpy(newBuckets, hashTable->buckets, oldSize * sizeof(Node*));
    free(hashTable->buckets);

    hashTable->buckets = newBuckets;
    hashTable->size = newSize;
}





// --- Concurrent Operations ---

Boolean conc_hashtable_insert(ConcurrentHashTable *hashTable, KeyType key, ValueType value) {
    HashValue hash = hash_key(key);
    unsigned bucket = hash % hashTable->size;
    Node *newNode = calloc(1, sizeof(Node));

    newNode->key = key;
    newNode->value = value;
    newNode->hash = hash_regular_key(hash);

    if (hashTable->buckets[bucket] == UNINITIALIZED) {
        initialize_bucket(hashTable, bucket);
    }

    Node *insertedNode = list_insert_hp(hashTable, bucket, newNode);
    if (insertedNode != newNode) {
        free(newNode);
        return FALSE;
    }

    __sync_fetch_and_add(&hashTable->count, 1);
    if ((float)hashTable->count / hashTable->size > 0.75f) {
        resize_table(hashTable);
    }

    return TRUE;
}

ValueType conc_hashtable_find(ConcurrentHashTable *hashTable, KeyType key) {
    HashValue hash = hash_key(key);
    unsigned bucket = hash % hashTable->size;

    if (hashTable->buckets[bucket] == UNINITIALIZED) {
        initialize_bucket(hashTable, bucket);
    }

    Node **prev;
    Node *foundNode = list_find_hp(hashTable, bucket, key, hash, &prev);
    if (foundNode && foundNode->key == key && foundNode->hash == hash) {
        return foundNode->value;
    }

    return NULL;
}

Boolean conc_hashtable_delete(ConcurrentHashTable *hashTable, KeyType key) {
    HashValue hash = hash_key(key);
    unsigned bucket = hash % hashTable->size;

    if (hashTable->buckets[bucket] == UNINITIALIZED) {
        initialize_bucket(hashTable, bucket);
    }

    Node *deletedNode = list_delete_hp(hashTable, bucket, key, hash);
    if (deletedNode) {
        __sync_fetch_and_sub(&hashTable->count, 1);
        return TRUE;
    }

    return FALSE;
}



// --- Main Function ---

int main() {
    // Initialize the hazard pointers key
    pthread_key_create(&hazard_pointer_key, free);

    // Create the hash table
    ConcurrentHashTable *hashTable = calloc(1, sizeof(ConcurrentHashTable));
    hashTable->size = 16; // Initial size of the hash table
    hashTable->buckets = calloc(hashTable->size, sizeof(Node*));
    hashTable->lockEnabled = FALSE;

    // Example usage of the hash table
    // Insertion
    conc_hashtable_insert(hashTable, (KeyType)1, (ValueType)"Value 1");
    conc_hashtable_insert(hashTable, (KeyType)2, (ValueType)"Value 2");

    // Finding
    ValueType value = conc_hashtable_find(hashTable, (KeyType)1);
    printf("Found value: %s\n", (char *)value);

    // Deletion
    if (conc_hashtable_delete(hashTable, (KeyType)1)) {
        printf("Key 1 deleted\n");
    }

    // Clean up
	int i = 0;
    for (i = 0; i < hashTable->size; i++) {
        Node *current = hashTable->buckets[i];
        while (current) {
            Node *temp = current;
            current = get_actual_node(current->markedNext);
            free(temp);
        }
    }
    free(hashTable->buckets);
    free(hashTable);

    return 0;
}
