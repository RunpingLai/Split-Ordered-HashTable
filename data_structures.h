#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdint.h>

typedef struct _node node_t;
typedef node_t *mark_ptr_t; // pointer to next node, with marked info
typedef unsigned int key_t;
typedef unsigned int value_t;
typedef struct _table hashtable_t;

struct _node
{
    key_t key;
    value_t value;
    mark_ptr_t marked_next;
};

struct _table
{
    unsigned int count; // Current Count of Nodes
    unsigned int size;  // Limit of Count
    mark_ptr_t *table;  // Array of pointers to hash table buckets
};

#endif