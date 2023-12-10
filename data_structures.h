#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdint.h>
#include <string.h>

typedef struct _node    node_t;
typedef node_t*         mark_ptr_t;     // pointer to next node, with marked info
typedef unsigned int    key_t;
typedef unsigned int    value_t;
typedef struct _table   hashtable_t;

#define LOCK_TABLE_SIZE 20000

/* * * * * * * * * * * * * * * * * * * * * *
 * lock-free hashtable data structures
 * * * * * * * * * * * * * * * * * * * * * */


/**
 * Struct representing a node in the lock-free list.
 */
struct _node {
    key_t       key;
    value_t     value;
    mark_ptr_t  marked_next;
};


/*
Struct representing the hash table.
*/
struct _table {
    
    unsigned int    count;             // Current Count of Nodes
    unsigned int    size;              // Limit of Count
    mark_ptr_t      *table;            // Array of pointers to hash table buckets
};


/* * * * * * * * * * * * * * * * * * * * * *
 * lock-based hashtable data structures
 * * * * * * * * * * * * * * * * * * * * * */


typedef struct lock_node
{
    key_t key;
    value_t value;
    struct lock_node *next;
} lock_node_t;

// Hashtable structure
typedef struct lock_hashtable
{
    lock_node_t *table[LOCK_TABLE_SIZE];
    pthread_mutex_t locks[LOCK_TABLE_SIZE];
} lock_hashtable_t;

#endif