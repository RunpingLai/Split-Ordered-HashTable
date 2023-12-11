#ifndef LOCK_BASED_HASHTABLE_H
#define LOCK_BASED_HASHTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "data_structures.h"




// Node structure for the linked list in each bucket


// Function to initialize the hashtable
lock_hashtable_t *lock_table_create();

// Simple hash function
unsigned int hash(my_key_t key);

// Insert function
bool lock_table_insert(lock_hashtable_t *ht, my_key_t key, value_t value);


// Lookup function
value_t lock_table_find(lock_hashtable_t *ht, my_key_t key);

// Function to free the hashtable
void lock_table_free(lock_hashtable_t *ht);

int lock_table_delete(lock_hashtable_t *ht, my_key_t key);


#endif