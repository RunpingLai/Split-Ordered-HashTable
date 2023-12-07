#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdint.h>

typedef unsigned int hash_t;
typedef void* my_key_t;
typedef void* value_t;

typedef struct _node node_t;
typedef node_t* mark_ptr_t;

struct _node {
    mark_ptr_t next;
    hash_t hash_code;
    my_key_t key;
    value_t value;
};

typedef struct {
    mark_ptr_t *table;
    unsigned count, size;
    int lock_value;
} conc_hashtable_t;

#endif