#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#include "resizable_lk_hashtable.h"
#include "data_structures.h"

#define R_LOAD_FACTOR 0.75


// Simple hash function
unsigned int r_hash(r_lock_hashtable_t* ht, key_t key) {
    return key % ht->size;
}

// Function to initialize the hashtable
r_lock_hashtable_t* r_lock_table_create() {
    r_lock_hashtable_t* ht = malloc(sizeof(r_lock_hashtable_t));
    ht->size = INIT_SIZE;
    ht->count = 0;
    ht->table = malloc(INIT_SIZE *sizeof(r_lock_node_t*));
    ht->locks = malloc(INIT_SIZE * sizeof(pthread_mutex_t));
    if (!ht) return NULL;

    for (int i = 0; i < ht->size; i++) {
        ht->table[i] = NULL;
        pthread_mutex_init(&ht->locks[i], NULL);
    }

    return ht;
}

// Insert function
bool r_lock_table_insert(r_lock_hashtable_t* ht, key_t key, value_t value) {
    unsigned int index = r_hash(ht, key);
    int res = 0;
    pthread_mutex_lock(&ht->locks[index]);
    r_lock_node_t* current = ht->table[index];

    // 查找键是否已经存在
    while (current != NULL) {
        if (current->key == key) {
            // 如果键已存在，更新值并返回
            current->value = value;
            pthread_mutex_unlock(&ht->locks[index]);
            return 1;
        }
        current = current->next;
    }

    // 如果键不存在，创建新节点并插入到链表头部
    r_lock_node_t* new_node = malloc(sizeof(r_lock_node_t));
    new_node->key = key;
    new_node->value = value;
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
    int new_count = __sync_fetch_and_add(&(ht->count) ,1);
    int cur_size = ht->size;
    if (((float)new_count / cur_size) > R_LOAD_FACTOR)
        res = r_lock_table_resize(ht);
    
    else
        pthread_mutex_unlock(&ht->locks[index]);
    return 1;
}


// Lookup function
value_t r_lock_table_find(r_lock_hashtable_t* ht, key_t key) {
    unsigned int index = r_hash(ht, key);
    pthread_mutex_lock(&ht->locks[index]);

    r_lock_node_t* temp = ht->table[index];
    while (temp != NULL) {
        if (temp->key == key) {
            pthread_mutex_unlock(&ht->locks[index]);
            return temp->value;
        }
        temp = temp->next;
    }

    pthread_mutex_unlock(&ht->locks[index]);
    return 0; // Assume 0 as default value
}

// Function to free the hashtable
void r_lock_table_free(r_lock_hashtable_t* ht) {
    for (int i = 0; i < LOCK_TABLE_SIZE; i++) {
        pthread_mutex_lock(&ht->locks[i]);
        r_lock_node_t* temp = ht->table[i];
        while (temp) {
            r_lock_node_t* to_delete = temp;
            temp = temp->next;
            free(to_delete);
        }
        pthread_mutex_unlock(&ht->locks[i]);
        pthread_mutex_destroy(&ht->locks[i]);
    }
    free(ht);
}

// TODO: Add value as parameter
int r_lock_table_delete(r_lock_hashtable_t* ht, key_t key)
{
    int index = key % LOCK_TABLE_SIZE;

    pthread_mutex_lock(&ht->locks[index]);

    r_lock_node_t* curr = ht->table[index];
    r_lock_node_t* prev = NULL;
        
    while (curr != NULL) {
        if (curr->key == key) {
            if (prev != NULL) {
                prev->next = curr->next;
            } else {
                ht->table[index] = curr->next;
            }
            free(curr);
            __sync_fetch_and_sub(&(ht->count), 1);
            pthread_mutex_unlock(&ht->locks[index]);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    pthread_mutex_unlock(&ht->locks[index]);
    return 0;
}

int r_lock_table_resize(r_lock_hashtable_t* ht) {
    int old_size = ht->size;
    int new_size = 2 * old_size;

    // Lock the whole table
    for (int i = 0; i < old_size; i++) {
        pthread_mutex_lock(&ht->locks[i]);
    }

    // Allocate new table
    r_lock_node_t** new_table = malloc(new_size * sizeof(r_lock_node_t*));
    pthread_mutex_t* new_locks = malloc(new_size * sizeof(pthread_mutex_t));

    // Initialize new locks
    for (int i = 0; i < new_size; i++) {
        pthread_mutex_init(&new_locks[i], NULL);
    }

    // Rehash the elements
    for (int i = 0; i < old_size; i++) {
        r_lock_node_t* current = ht->table[i];
        while (current != NULL) {
            int index = current->key % new_size;
            r_lock_node_t* next = current->next;

            // Insert into new table
            current->next = new_table[index];
            new_table[index] = current;

            current = next;
        }
    }

    // Free old locks and table, but keep the nodes
    free(ht->locks);
    free(ht->table);

    // Update hashtable with new table and locks
    ht->table = new_table;
    ht->locks = new_locks;
    ht->size = new_size;

    // Unlock the whole table
    for (int i = 0; i < new_size; i++) {
        pthread_mutex_unlock(&new_locks[i]);
    }
    return 1;
}


// int main() {
//     lock_hashtable_t* ht = init_hashtable();

//     // Example usage
//     lock_table_insert(ht, 1, 100);
//     lock_table_insert(ht, 2, 200);
//     lock_table_insert(ht, 201, 20100);

//     printf("Value at key 1: %u\n", lock_table_find(ht, 1));
//     printf("Value at key 2: %u\n", lock_table_find(ht, 2));
//     printf("Value at Key 201: %u\n", lock_table_find(ht, 201));
//     printf("%u\n", ht->table[1]->value);

//     lock_table_free(ht);
//     return 0;
// }
