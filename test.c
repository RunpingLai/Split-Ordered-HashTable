#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>     // rand, srand
#include <stdbool.h>
#include <string.h>

#include "data_structures.h"
#include "lock_based_hashtable.h"
#include "so_list_hashtable.h"
#include "resizable_lk_hashtable.h"

// if 0 means testing lock-based hashtable
// 0 SO
// 1 LK
// 2 RSZ_LK
#define TEST_FLAG 1
#if TEST_FLAG == 0
#define hashtable_t hashtable_t
#define tbl_find    table_find
#define tbl_delete  table_delete
#define tbl_insert  table_insert
#define tbl_create  table_create
#define tbl_free    table_free
#elif TEST_FLAG == 1
#define hashtable_t lock_hashtable_t
#define tbl_find    lock_table_find
#define tbl_delete  lock_table_delete
#define tbl_insert  lock_table_insert
#define tbl_create  lock_table_create
#define tbl_free    lock_table_free
#else
#define hashtable_t r_lock_hashtable_t
#define tbl_find    r_lock_table_find
#define tbl_delete  r_lock_table_delete
#define tbl_insert  r_lock_table_insert
#define tbl_create  r_lock_table_create
#define tbl_free    r_lock_table_free
#endif

hashtable_t*        tbl;
// lock_hashtable_t*   tbl;


#define LOWER 50000
#define UPPER 20000000
// #define LOWER 500
// #define UPPER 20000
#define THREADS 32
// OPS_COUNT is a multiple of 2 since the thread counts are a factor of 2
#define OPS_COUNT 16777216
//4096, 32768, 524288, 1048576, 4194304, 8388608, 16777216


static uint64_t random_search_key() {
    return rand() % UPPER;
}


static uint64_t random_insert_key() {
    return (rand() % 
           (UPPER - LOWER)) + LOWER;
}


static uint64_t random_delete_key() {
    return rand() % LOWER;
}


static void *test_case_1(void *arg) {
    // 50% inserts, 0% finds, 50% deletes
    bool status;
    int iterations = *(int*)arg;
    int i;
    iterations /= 2;
    
    my_key_t key_i, key_d, temp_val;
    value_t val;

    for (i = 0; i < iterations; i++) {
        // printf("iteration:%d\n", i);
        key_i = random_insert_key();
        temp_val = key_i >> 1;
        val = (void*)&temp_val;
        status = tbl_insert(tbl, key_i, val);
        key_d = random_delete_key();
        status = tbl_delete(tbl, key_d);
    }
}

static void *test_case_2(void *arg) {
    // 33% inserts, 33% finds, 33% deletes
    bool status;
    int iterations = *(int*)arg;
    int i;
    iterations /= 3;
    
    my_key_t key_i, key_d, key_f, temp_val;
    value_t val, result_val;

    for (i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        temp_val = key_i >> 1;
        val = (void*)&temp_val;
        status = tbl_insert(tbl, key_i, val);
        key_f = random_search_key();
        result_val = tbl_find(tbl, key_f);
        key_d = random_delete_key();
        status = tbl_delete(tbl, key_d);
    }
}

static void *test_case_3(void *arg) {
    // 25% inserts, 50% finds, 25% deletes
    bool status;
    int iterations = *(int*)arg;
    int i, j;
    iterations /= 4;
    
    my_key_t key_i, key_f, key_d, temp_val;
    value_t val, result_val;

    for (i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        temp_val = key_i >> 1;
        val = (void*)&temp_val;
        status = tbl_insert(tbl, key_i, val);
        for j = 0; j < 2; j++) {
            key_f = random_search_key();
            result_val = tbl_find(tbl, key_f);
        }
        key_d = random_delete_key();
        status = tbl_delete(tbl, key_d);
    }
}

static void *test_case_4(void *arg) {
    // 15% inserts, 70% finds, 15% deletes
    bool status;
    int iterations = *(int*)arg;
    int i, j;
    iterations /= 7;
    
    my_key_t key_i, key_f, key_d, temp_val;
    value_t val, result_val;

    for (i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        temp_val = key_i >> 1;
        val = (void*)&temp_val;
        status = tbl_insert(tbl, key_i, val);
        for (j = 0; j < 5; j++) {
            key_f = random_search_key();
            result_val = tbl_find(tbl, key_f);
        }
        key_d = random_delete_key();
        status = tbl_delete(tbl, key_d);
    }
}

static void *test_case_5(void *arg) {
    // 5% inserts, 90% finds, 5% deletes
    bool status;
    int iterations = *(int*)arg;
    int i, j;
    iterations /= 20;
    
    my_key_t key_i, key_f, key_d, temp_val;
    value_t val, result_val;

    for (i = 0; i < iterations; i++) {
        key_i = random_insert_key();
        temp_val = key_i >> 1;
        val = (void*)&temp_val;
        status = tbl_insert(tbl, key_i, val);
        for (j = 0; j < 18; j++) {
            key_f = random_search_key();
            result_val = tbl_find(tbl, key_f);
        }
        key_d = random_delete_key();
        status = tbl_delete(tbl, key_d);
    }
}

struct timespec timer_start() {
    struct timespec start_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    return start_time;
}

double timer_elapsed(struct timespec start_time) {
    struct timespec end_time;
    clock_gettime(CLOCK_REALTIME, &end_time);
    return (((double) (end_time.tv_sec - start_time.tv_sec)) +  /* sec */
            ((double)(end_time.tv_nsec - start_time.tv_nsec))/1000000000); /*nanosec */
}



int main()
{
    // seeding so that different random sequences get generated
    srand(time(NULL));

    pthread_t threads[32];
    int thread_num;
    int t;
    int test_case_num;
    void *(*test_case_fn_ptr)(void*);
    

    for (test_case_num=1; test_case_num<=5; test_case_num++) {
        switch (test_case_num)
        {
        case 1:
            test_case_fn_ptr = test_case_1;
            break;

        case 2:
            test_case_fn_ptr = test_case_2;
            break;

        case 3:
            test_case_fn_ptr = test_case_3;
            break;

        case 4:
            test_case_fn_ptr = test_case_4;
            break;
        
        case 5:
            test_case_fn_ptr = test_case_5;
            break;
        default:
            printf("No Such Test Case, Exiting\n");
            return 0;
        }

        printf("-------------------Test Case %d Start-------------------\n", test_case_num);
        // 1 2 4 8 16 32
        int thread_num;
        for (thread_num=1; thread_num<=THREADS; thread_num*=2) {
            tbl = tbl_create();
            int each_thread_ops_count = OPS_COUNT / thread_num;

            struct timespec start = timer_start();
            for (t=0; t<thread_num; t++) 
                pthread_create(&threads[t], NULL, test_case_fn_ptr, &each_thread_ops_count);

            for (t=0; t<thread_num; t++) 
                pthread_join(threads[t], NULL);
            
            double benchmark_time = timer_elapsed(start);
            printf("%f, ", benchmark_time);
            printf("threads: %d, time: %f\n", thread_num, benchmark_time);

            tbl_free(tbl);
        }
        printf("-------------------Test Case %d Ended-------------------\n", test_case_num);
    }


    return 1;
}
