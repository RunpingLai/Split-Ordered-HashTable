#include <pthread.h>
#include <stdio.h>
#include <unistd.h> // sleep
#include "data_structures.h"
#include "so_list_hashtable.h"

void print_list(mark_ptr_t * head){
        
    node_t * curr=(node_t *)get_pointer((mark_ptr_t)*head);
    while (curr){
        my_key_t normal_key = curr->key/2;
        value_t normal_val = curr->value;
        normal_key = normal_key*2;
        
        normal_key = reverse32bits(normal_key);
        
        if (curr->key%2==1){
            printf("Key : %d, Value : %d\n",normal_key, normal_val);
        }
        
        else printf("*%d \n",normal_key);
        curr=(node_t *)get_pointer(curr->marked_next);
    }
}

hashtable_t* HT;

#define INSERT_CNT 1000000
void*
async_insert (void* args)
{
    int* arg = (int*) args;
    int base = INSERT_CNT * (*arg); // 0 100 200 300
    int i;
    for (i = 0 ; i < INSERT_CNT; i++) {
        table_insert(HT, (my_key_t)(base + i), (value_t)(1));
        table_insert(HT, (my_key_t)(base + i - INSERT_CNT), (value_t)(2));
    }
    return NULL;
    
}

/* ABA Problem Test
 */

void*
thread1() {
    // read key : 58
    int firstReadValue = table_find(HT, 58);
    
    sleep(1);
    
    // check if is modified
    if (firstReadValue == table_find(HT, 58)) {
        printf("Didn't detect modification, meaning ABA problem exists\n");
    } else {
        printf("ABA problem solved\n");
    }
    return 0;
}

void*
thread2() {
    // A
    // B
    table_delete(HT, 58);
    printf("58 Deleted\n");
    // A
    table_insert(HT, 58, 66058);
    return 0;
}

int main()
{
    int i;
    pthread_t threads[4];
    /* * * * * * * * * * * * * * * * * * * * * *
     * 1. ABA Test
     * * * * * * * * * * * * * * * * * * * * * */
    
    HT = table_create();
    table_insert(HT, 58, 66058);
    pthread_create(&threads[0], NULL, thread1, NULL);
    pthread_create(&threads[1], NULL, thread2, NULL);
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    table_free(HT);

    /* * * * * * * * * * * * * * * * * * * * * *
     * 2. Simple Insert & Print
     * * * * * * * * * * * * * * * * * * * * * */
    hashtable_t* tbl = table_create();
    for(i=0;i<32;i++)
        table_insert(tbl, i, i+66000);
    print_list(&tbl->table[0]);
    printf("Table  Size:%d\n", tbl->size);
    printf("Table Count:%d\n", tbl->count);
    table_free(tbl);
    

    /* * * * * * * * * * * * * * * * * * * * * *
     * 3. Thread x 4
     * * * * * * * * * * * * * * * * * * * * * */
    
    HT = table_create();
    
    for(i=0; i<4; i++) 
        pthread_create(&threads[i], NULL, async_insert, &i);
    for(i=0; i<4; ++i)
		pthread_join (threads [i], NULL);

    printf("Elements Count : %d\n", HT->count);
    table_free(HT);

    return 1;
}