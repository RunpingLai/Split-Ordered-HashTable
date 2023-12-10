#ifndef SO_LIST_HASHTABLE_H
#define SO_LIST_HASHTABLE_H

/* Split Ordered Lists: Extensible Hash Tables */

#include "data_structures.h"

// reverse the bits of a 32-bit unsigned int
unsigned
reverse32bits(unsigned x);

// produce keys according to split ordering
key_t so_regularkey(key_t key);

key_t so_dummykey(key_t key);

mark_ptr_t
get_count(mark_ptr_t a);

mark_ptr_t
get_pointer(mark_ptr_t a);

mark_ptr_t
set_count(
    mark_ptr_t a, 
    mark_ptr_t count
);

mark_ptr_t
set_pointer(
    mark_ptr_t a, 
    mark_ptr_t ptr
);

mark_ptr_t
set_both(
    mark_ptr_t a, 
    mark_ptr_t ptr, 
    mark_ptr_t count
);

// // thread-private variables
// __thread mark_ptr_t *prev;
// __thread mark_ptr_t curr;
// __thread mark_ptr_t next;

// mark_ptr_t *Head = 0;

int list_find(
    mark_ptr_t **head, 
    key_t key
);

int list_insert(
    mark_ptr_t *head, 
    node_t *node
);

int list_delete(
    mark_ptr_t *head, 
    key_t key
);

// get the parent of a bucket by just unseting the MSB
int get_parent(int bucket);

void initialize_bucket(
    hashtable_t *ht, 
    int bucket
);

int table_find(
    hashtable_t *ht, 
    key_t key
);

int table_insert(
    hashtable_t *ht, 
    key_t key, 
    value_t value
);

int table_delete(
    hashtable_t *ht, 
    key_t key
);

hashtable_t*
table_create();

void table_free(hashtable_t *ht);

#endif