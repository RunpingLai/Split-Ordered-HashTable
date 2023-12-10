#ifndef LIST_OPERATIONS_H
#define LIST_OPERATIONS_H

#include "data_structures.h"
#include <stdlib.h>

// List operation function declarations
typedef struct _table table_t;

/*reverse the bits of a 32-bit unsigned int
 @param x The integer whose bits are to be reversed.
 @return The 32-bit unsigned integer with its bits reversed.
 */
unsigned reverse32bits(unsigned x) {
   static unsigned char table[256] = {
   0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
   0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
   0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
   0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
   0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
   0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
   0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
   0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
   0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
   0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
   0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
   0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
   0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
   0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
   0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
   0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};
   int i;
   unsigned r;

   r = 0;
   for (i = 3; i >= 0; i--) {
      r = (r << 8) + table[x & 0xFF];
      x = x >> 8;
   }
   return r;
}

//produce keys according to split ordering
key_t so_regularkey(key_t key){
    return reverse32bits(key|0x80000000);
}

/*
Produce dummy keys according to split ordering.
*/
key_t so_dummykey(key_t key){
    return reverse32bits(key);
}


mark_ptr_t get_count (mark_ptr_t a)
{
    mark_ptr_t b = (unsigned long long)a >> 63;
    return b;
}


mark_ptr_t get_pointer (mark_ptr_t a)
{
    mark_ptr_t b = (unsigned long long)a << 1;
    b = (unsigned long long)b >> 1;
    return b;
}


mark_ptr_t set_count (mark_ptr_t a, mark_ptr_t count)
{
    unsigned long long count_temp = (unsigned long long)count << 63;
    mark_ptr_t b = get_pointer(a);
    b = (unsigned long long)b | count_temp;
    return b;
}

mark_ptr_t set_pointer(mark_ptr_t a, mark_ptr_t ptr){
    mark_ptr_t b = 0;
    mark_ptr_t c = get_count(a);
    b = set_count(b,c);
    ptr = get_pointer(ptr);
    b= (unsigned long long)b | (unsigned long long)ptr;
    return b;
}

mark_ptr_t set_both(mark_ptr_t a,mark_ptr_t ptr, mark_ptr_t count){
    a=set_pointer(a,ptr);
    a=set_count(a,count);
    return a;
}

// thread-private variables
__thread mark_ptr_t * prev;
__thread mark_ptr_t curr;
__thread mark_ptr_t next;

mark_ptr_t * Head = 0;
/*
 Search for a key in the list starting from the head node.
 @return 0 if the key is not found, 1 if the key is found and not marked for deletion, or 2 if the key is found but marked for deletion.
 */
int list_find(mark_ptr_t ** head, key_t key)
{
try_again:
    prev = (mark_ptr_t *) * head;
    curr = set_both(curr, get_pointer(*prev), get_count(*prev));
    while (1){
        if (get_pointer(curr) == 0)
            return 0;

        // printf("%d\n", get_pointer(curr));
        mark_ptr_t pointer=get_pointer(((mark_ptr_t)get_pointer(curr))->marked_next);
        mark_ptr_t mark_bit = get_count(((mark_ptr_t)get_pointer(curr))->marked_next);

        next = set_both(next, pointer, mark_bit);
        key_t ckey = ((mark_ptr_t) get_pointer(curr))->key;
        mark_ptr_t check = set_both(check, curr, 0);
        if ((*prev) != check)
            goto try_again;
        if (get_count(next) == 0) {
            if (ckey >= key)
                return (ckey == key);
            prev = &(((mark_ptr_t)get_pointer(curr))->marked_next);
        }

        else {
            mark_ptr_t compare_value = set_both(compare_value, curr, 0);
            mark_ptr_t new_value = set_both (new_value, next, 0);

            if (__sync_bool_compare_and_swap(prev, compare_value, new_value)){
                free((mark_ptr_t)get_pointer(curr));
            }
            else goto try_again;
        }

        curr = set_both(curr, next, get_count(next));
    }
}

int list_insert(mark_ptr_t * head, node_t* node)
{
    int res;
    int temp = 0;
    key_t key = node->key;
    value_t value = node->value; // useless

    while (1) {
        if (list_find(&head, key))
            return 0;

        node->marked_next = set_both(node->marked_next, get_pointer(curr), 0);
        mark_ptr_t compare_value = set_both(compare_value,get_pointer(curr),0);
        mark_ptr_t new_value = set_both(new_value,(mark_ptr_t ) node,0);

        temp++;
        res =__sync_bool_compare_and_swap(prev,compare_value,new_value);
        if (res){
            //Head=head;
            return 1;
        }
    }
}

int list_delete(mark_ptr_t *head ,key_t key){
    
    while (1){
        if (!list_find(&head,key))  return 0;
        mark_ptr_t compare_value = set_both(compare_value,get_pointer(next),0);
        mark_ptr_t new_value = set_both(new_value,get_pointer(next),1);

        if(!__sync_bool_compare_and_swap(&(((node_t *)get_pointer(curr))->marked_next),compare_value,new_value)) 
            continue;


        compare_value = set_both(compare_value,get_pointer(curr),0);
        new_value = set_both(new_value,get_pointer(next),0);

        if(__sync_bool_compare_and_swap(prev,compare_value,new_value))
            free((node_t *)get_pointer(curr));

        else list_find(&head,key);
        //Head=head;//TODO: thats not very safe
        return 1;

    }
}
#endif