#ifndef RESIZABLE_LK_HASHTABLE_H
#define RESIZABLE_LK_HASHTABLE_H


#include "data_structures.h"

// Simple hash function
unsigned int r_hash(r_lock_hashtable_t *ht, my_key_t key);

// Function to initialize the hashtable
r_lock_hashtable_t *r_lock_table_create();
// Insert function
bool r_lock_table_insert(r_lock_hashtable_t *ht, my_key_t key, value_t value);

// Lookup function
value_t r_lock_table_find(r_lock_hashtable_t *ht, my_key_t key);
// Function to free the hashtable
void r_lock_table_free(r_lock_hashtable_t *ht);

// TODO: Add value as parameter
int r_lock_table_delete(r_lock_hashtable_t *ht, my_key_t key);

int r_lock_table_resize(r_lock_hashtable_t *ht);


#endif