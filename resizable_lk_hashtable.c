#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#include "resizable_lk_hashtable.h"
#include "data_structures.h"

#define R_LOAD_FACTOR 0.75

// Simple hash function
unsigned int r_hash(r_lock_hashtable_t* ht, my_key_t key) {
    return key % ht->size;
}

// Function to initialize the hashtable
r_lock_hashtable_t* r_lock_table_create() {
    r_lock_hashtable_t* ht = malloc(sizeof(r_lock_hashtable_t));
    ht->size = INIT_SIZE;
    ht->count = 0;
    ht->table = malloc(INIT_SIZE * sizeof(r_lock_node_t*));
    ht->locks = malloc(INIT_SIZE * sizeof(pthread_mutex_t));
    if (!ht) return NULL;
    int i;
    for (i = 0; i < ht->size; i++) {
        ht->table[i] = NULL;
        pthread_mutex_init(&ht->locks[i], NULL);
    }

    return ht;
}

// Insert function
bool r_lock_table_insert(r_lock_hashtable_t* ht, my_key_t key, value_t value) {
    unsigned int index;
    int ret, res;
// tryagain:
    int old_size = ht->size;
    index = r_hash(ht, key);
    res = 0;
    pthread_mutex_lock(&ht->locks[index]);
    // if (ret != 0)
    //     goto tryagain;
    // pthread_mutex_lock(&ht->locks[index]);
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

    pthread_mutex_unlock(&ht->locks[index]);

    int new_count = __sync_fetch_and_add(&(ht->count) ,1);
    int cur_size = ht->size;
    // printf("new_count:%d, cur_size:%d\n", new_count, cur_size);
    if (((float)new_count / cur_size) > R_LOAD_FACTOR){
        // printf("resizing\n");
        res = r_lock_table_resize(ht);
    }
        

    // else
    //     pthread_mutex_unlock(&ht->locks[index]);
    return 1;
}


// Lookup function
value_t r_lock_table_find(r_lock_hashtable_t* ht, my_key_t key) {
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
    int i;
    for (i = 0; i < ht->size; i++) {
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
int r_lock_table_delete(r_lock_hashtable_t* ht, my_key_t key)
{
    int index;
    int ret;
// tryagain:
    index = key % ht->size;

    // ret = pthread_mutex_trylock(&ht->locks[index]);
    // if (ret != 0) 
    //     goto tryagain;
    pthread_mutex_lock(&ht->locks[index]);

    r_lock_node_t* curr = ht->table[index];
    r_lock_node_t* prev = NULL;
        
    while (curr != NULL) {
        // printf("%d\n", curr == NULL);
        // printf("%d\n", curr->key);
        if (curr->key == key) {
            if (prev != NULL) {
                prev->next = curr->next;
            } else {
                ht->table[index] = curr->next;
            }
            
            __sync_fetch_and_sub(&(ht->count), 1);
            free(curr);
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
    int i;

    // Lock the whole table
    for (i = 0; i < old_size; i++) {
        pthread_mutex_lock(&ht->locks[i]);
    }

    // Allocate new table
    r_lock_node_t** new_table = malloc(new_size * sizeof(r_lock_node_t*));
    pthread_mutex_t* new_locks = malloc(new_size * sizeof(pthread_mutex_t));

    for (i = 0; i < new_size; i++) {
        new_table[i] = NULL;
        // pthread_mutex_init(&ht->locks[i], NULL);
    }

    // Initialize new locks
    for (i = 0; i < new_size; i++) {
        pthread_mutex_init(&new_locks[i], NULL);
    }

    // Rehash the elements
    for (i = 0; i < old_size; i++) {
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
    for (i = 0; i < new_size; i++) {
        pthread_mutex_unlock(&new_locks[i]);
    }
    // printf("resizing done\n");
    return 1;
}


// int main() {
//     r_lock_hashtable_t* ht = r_lock_table_create();

//     // Example usage
//     // r_lock_table_insert(ht, 1, 100);
//     // r_lock_table_insert(ht, 2, 200);
//     // r_lock_table_insert(ht, 201, 20100);

//     // printf("Value at key 1: %u\n", r_lock_table_find(ht, 1));
//     // printf("Value at key 2: %u\n", r_lock_table_find(ht, 2));
//     // printf("Value at Key 201: %u\n", r_lock_table_find(ht, 201));
//     // printf("%u\n", ht->table[1]->value);

//     int i;
//     for (i=0; i<500; i++){
//         r_lock_table_insert(ht, i, i*100);
//     }
    
//     // for (i=0; i<32; i++){
//     //     printf("ht->table[%d]:%d\n",i,ht->table[i]->value);
//     // }

//     for (i=0; i<32; i++)
//         r_lock_table_delete(ht, i);

//     r_lock_table_free(ht);
//     return 0;
// }
