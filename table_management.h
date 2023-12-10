#ifndef TABLE_MANAGEMENT_H
#define TABLE_MANAGEMENT_H

#include "data_structures.h"
#include "list_operations.h"
#include <cstddef>

// Table management function declarations
//get the parent of a bucket by just unseting the MSB
int get_parent(int bucket){
     int msb = 1<< ((sizeof(int)*8)-__builtin_clz(bucket)-1);
     int result = bucket & ~msb;
     return result;
}    

void initialize_bucket(table_t *ht, int bucket)
{
    int parent = get_parent(bucket);
    // printf("parent:%d, bucket:%d\n", parent, bucket);

    if (ht->table[parent] == NULL)
        initialize_bucket(ht, parent);

    node_t *dummy = (node_t*)malloc(sizeof(node_t));
    dummy->key = so_dummykey(bucket);
    if (!list_insert(&(ht->table[parent]), dummy)) {
        free(dummy);
        dummy = (mark_ptr_t) get_pointer(curr);
    }
    ht->table[bucket] = (mark_ptr_t) dummy;
}

int table_find(table_t *ht, key_t key)
{
    int bucket = key % ht->size;
    if (ht->table[bucket] == NULL)
        initialize_bucket (ht, bucket);

    mark_ptr_t * temp = &(ht->table[bucket]);
    return list_find(&temp, so_regularkey(key));
}

int table_insert(table_t *ht, key_t key, value_t value)
{
    node_t *node = (node_t *) malloc (sizeof(node_t));
    node->key = so_regularkey(key);
    node->value = value;
    int bucket = key % ht->size;
    // printf("bucket:%d, key:%d\n", bucket, key);

    if (ht->table[bucket] == NULL)
        initialize_bucket(ht, bucket);
    
    if (!list_insert(&(ht->table[bucket]), node)) {
        free(node);
        return 0;
    }

    int curr_size = ht->size;
    int new_count = __sync_fetch_and_add(&(ht->count) ,1);
    if (((float)new_count / curr_size) > LOAD_FACTOR) {
        node_t ** old_table = (ht->table); //&ht->table
        node_t ** new_table = calloc (sizeof (node_t*), curr_size * 2);
        memcpy (new_table, old_table, sizeof (node_t*) * curr_size);
        if (!__sync_bool_compare_and_swap(&(ht->size), curr_size, 2*curr_size)) {
            free(new_table);
            return 0;
        }
        if (!__sync_bool_compare_and_swap((void**)&ht->table, old_table, new_table))
            free(new_table);
        // int res = __sync_bool_compare_and_swap(&(ht->size), curr_size, 2*curr_size);
    }
    return 1;
}

int table_delete(table_t *ht, key_t key)
{
    int bucket = key % ht->size;
    if (ht->table[bucket] == NULL)
        initialize_bucket(ht, bucket);
    
    if (!list_delete(&(ht->table[bucket]), so_regularkey(key)))
        return 0;
    
    int res = __sync_fetch_and_sub(&(ht->count), 1);
    return 1;
}

table_t* table_create()
{
    table_t* ht = (table_t*) malloc (sizeof(table_t));
    ht->size = 16;
    ht->count = 0;
    ht->table = calloc (sizeof(node_t*), 16);
    ht->table[0] = calloc (sizeof(node_t), 1);
    // ht->table[0]->key = 0;
    return ht;
}


void table_free(table_t *ht) {
    if (ht == NULL) return;


    // node_t * head = ht->table[0];
    // 遍历每个桶
    node_t * current = ht->table[0];
    while (current != NULL) {
        node_t * next = (node_t *) get_pointer(current->marked_next);
        free(current);
        current = next;
    }
    // for (int i = 0; i < ht->size; i++) {
    //     node_t * current = ht->table[i];
    //     while (current != NULL) {
    //         if (current->marked_next != NULL){
    //             // 保存下一个节点的指针
    //             mark_ptr_t next = (mark_ptr_t) get_pointer(current->marked_next);
    //         }
    //         // 释放当前节点
    //         printf("%d\n", current->key);
    //         free(current);
    //         // 移动到下一个节点
    //         current = next;
    //     }
    // }

    // 释放存储桶指针的数组
    free(ht->table);
    // 释放哈希表结构
    free(ht);
}



#endif