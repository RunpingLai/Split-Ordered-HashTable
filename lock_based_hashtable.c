#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#include "lock_based_hashtable.h"
#include "data_structures.h"



// Simple hash function
unsigned int hash(my_key_t key) {
    return key % LOCK_TABLE_SIZE;
}

// Function to initialize the hashtable
lock_hashtable_t* lock_table_create() {
    lock_hashtable_t* ht = malloc(sizeof(lock_hashtable_t));
    if (!ht) return NULL;

    for (int i = 0; i < LOCK_TABLE_SIZE; i++) {
        ht->table[i] = NULL;
        pthread_mutex_init(&ht->locks[i], NULL);
    }

    return ht;
}

// Insert function
bool lock_table_insert(lock_hashtable_t* ht, my_key_t key, value_t value) {
    unsigned int index = hash(key);

    pthread_mutex_lock(&ht->locks[index]);
    lock_node_t* current = ht->table[index];

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
    lock_node_t* new_node = malloc(sizeof(lock_node_t));
    new_node->key = key;
    new_node->value = value;
    new_node->next = ht->table[index];
    ht->table[index] = new_node;

    pthread_mutex_unlock(&ht->locks[index]);
    return 1;
}


// Lookup function
value_t lock_table_find(lock_hashtable_t* ht, my_key_t key) {
    unsigned int index = hash(key);
    pthread_mutex_lock(&ht->locks[index]);

    lock_node_t* temp = ht->table[index];
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
void lock_table_free(lock_hashtable_t* ht) {
    for (int i = 0; i < LOCK_TABLE_SIZE; i++) {
        pthread_mutex_lock(&ht->locks[i]);
        lock_node_t* temp = ht->table[i];
        while (temp) {
            lock_node_t* to_delete = temp;
            temp = temp->next;
            free(to_delete);
        }
        pthread_mutex_unlock(&ht->locks[i]);
        pthread_mutex_destroy(&ht->locks[i]);
    }
    free(ht);
}

// TODO: Add value as parameter
int lock_table_delete(lock_hashtable_t* ht, my_key_t key)
{
    int index = key % LOCK_TABLE_SIZE;

    pthread_mutex_lock(&ht->locks[index]);

    lock_node_t* curr = ht->table[index];
    lock_node_t* prev = NULL;
        
    while (curr != NULL) {
        if (curr->key == key) {
            if (prev != NULL) {
                prev->next = curr->next;
            } else {
                ht->table[index] = curr->next;
            }
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
