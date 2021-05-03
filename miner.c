/**
 * @file mine.c
 *
 * Parallelizes the hash inversion technique used by cryptocurrencies such as
 * bitcoin.
 *
 * Input:    Number of threads, block difficulty, and block contents (string)
 * Output:   Hash inversion solution (nonce) and timing statistics.
 *
 * Compile:  (run make)
 *           When your code is ready for performance testing, you can add the
 *           -O3 flag to enable all compiler optimizations.
 *
 * Run:      ./miner 4 24 'Hello CS 521!!!'
 *
 *   Number of threads: 4
 *     Difficulty Mask: 00000000000000000000000011111111
 *          Block data: [Hello CS 521!!!]
 *
 *   ----------- Starting up miner threads!  -----------
 *
 *   Solution found by thread 3:
 *   Nonce: 10211906
 *   Hash: 0000001209850F7AB3EC055248EE4F1B032D39D0
 *   10221196 hashes in 0.26s (39312292.30 hashes/sec)
 */

#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "elist.h"
#include "sha1.h"
#include "logger.h"

#define MAX_INPUT_LENGTH (10)
#define TASK_RANGE (110)
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condc = PTHREAD_COND_INITIALIZER;
pthread_cond_t condp = PTHREAD_COND_INITIALIZER;

int final_thread;
uint8_t final_result_digest[SHA1_HASH_SIZE];
struct elist *task_list; 
uint64_t final_result_nonce = 0;
unsigned long long total_inversions;
typedef struct thread_struct {
    uint32_t difficulty_mask;
    char *bitcoin_block_data;
    int thread_id;
} t_struct;

double get_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void create_task(uint64_t first_nonce_in_range) {
    elist_add(task_list, &first_nonce_in_range);
}

void print_binary32(uint32_t num) {
    int i;
    for (i = 31; i >= 0; --i) {
        uint32_t position = (unsigned int) 1 << i;
        printf("%c", ((num & position) == position) ? '1' : '0');
    }
    puts("");
}

uint64_t mine(char *data_block, uint32_t difficulty_mask,
        uint64_t nonce_start, uint64_t nonce_end,
        uint8_t digest[SHA1_HASH_SIZE]) {

    for (uint64_t nonce = nonce_start; nonce < nonce_end; nonce++) {
        /* A 64-bit unsigned number can be up to 20 characters  when printed: */
        size_t buf_sz = sizeof(char) * (strlen(data_block) + 20 + 1);
        char *buf = malloc(buf_sz);

        /* Create a new string by concatenating the block and nonce string.
         * For example, if we have 'Hello World!' and '10', the new string
         * is: 'Hello World!10' */
        snprintf(buf, buf_sz, "%s%lu", data_block, nonce);

        /* Hash the combined string */
        sha1sum(digest, (uint8_t *) buf, strlen(buf));
        free(buf);
        total_inversions++;

        /* Get the first 32 bits of the hash */
        uint32_t hash_front = 0;
        hash_front |= digest[0] << 24;
        hash_front |= digest[1] << 16;
        hash_front |= digest[2] << 8;
        hash_front |= digest[3];

        /* Check to see if we've found a solution to our block */
        if ((hash_front & difficulty_mask) == hash_front) {
            return nonce;
        }
    }
    return 0;
}

void *consumer_thread(void *ptr) {
    struct thread_struct* thread_data = (struct thread_struct*) ptr;

    while (true) {
        pthread_mutex_lock(&mutex);
        while (elist_size(task_list) == 0 && final_result_nonce == 0) { 
            pthread_cond_wait(&condc, &mutex);
        }
        if (final_result_nonce != 0) {
            pthread_mutex_unlock(&mutex);
            return 0;
        }
        u_int64_t *p_task_consumed = elist_get(task_list, 0);
        u_int64_t task_consumed = *p_task_consumed;
        elist_remove(task_list, 0);

        /* Wake up the producer */
        pthread_cond_signal(&condp); // send signal hey I am consuming
        pthread_mutex_unlock(&mutex); // unlock

        uint8_t digest_con[SHA1_HASH_SIZE];

        /* Mine the block. */
        uint64_t nonce = mine(
                thread_data->bitcoin_block_data,
                thread_data->difficulty_mask,
                task_consumed, task_consumed + TASK_RANGE,
                digest_con);
        
        if (nonce != 0) {
            pthread_mutex_lock(&mutex);
            if (final_result_nonce == 0 || final_result_nonce > nonce) {
                final_result_nonce = nonce;
            }
            final_thread = thread_data->thread_id;
            memcpy(final_result_digest, digest_con, sizeof(digest_con));
            pthread_cond_signal(&condp);
            pthread_mutex_unlock(&mutex);
            break;
        }
    }
    return 0;
}

int get_strtol(char *num)
{
    char *rest;
    long threads = strtol(num, &rest, MAX_INPUT_LENGTH);
    return (int) threads;
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Usage: %s threads difficulty 'block data (string)'\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *str_threads = argv[1];
    int num_threads = get_strtol(str_threads);
    if (num_threads == 0) {
        return EXIT_FAILURE;
    }
    printf("Number of threads: %d\n", num_threads);

    // Allow the user to specify anywhere between 1 and 32 bits of zeros.
    uint32_t difficulty_mask = 0x00000FFF;
    char *str_n_leading_zeros = argv[2];
    uint32_t nLeadingZeros = get_strtol(str_n_leading_zeros);
    difficulty_mask = (1 << (32 - nLeadingZeros)) - 1;
    printf("  Difficulty Mask: ");
    print_binary32(difficulty_mask);

    /* We use the input string passed in (argv[3]) as our block data. In a
     * complete bitcoin miner implementation, the block data would be composed
     * of bitcoin transactions. */
    char *bitcoin_block_data = argv[3];
    printf("       Block data: [%s]\n", bitcoin_block_data);

    printf("\n----------- Starting up miner threads!  -----------\n\n");

    task_list = elist_create(num_threads * 5, sizeof(uint64_t));

    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    t_struct **arg_thread = malloc(sizeof(t_struct*) * num_threads);

    for (int i = 0; i < num_threads; ++i) {    
        arg_thread[i] = malloc(sizeof(t_struct));
        arg_thread[i]->difficulty_mask = difficulty_mask;
        arg_thread[i]->bitcoin_block_data = bitcoin_block_data;
        arg_thread[i]->thread_id = i;
        pthread_create(&threads[i], NULL, consumer_thread, (void *)arg_thread[i]);
    }

    double start_time = get_time();

    u_int64_t curr_nonce = 1;
    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (elist_size(task_list) == elist_capacity(task_list)
            && final_result_nonce == 0) { // while list is full keep waiting
            pthread_cond_wait(&condp, &mutex); // sleep until they consume it
        }
        if (final_result_nonce == 0) {
            create_task(curr_nonce);
            curr_nonce += TASK_RANGE;
            pthread_cond_signal(&condc);
            pthread_mutex_unlock(&mutex);
        } else {
            pthread_cond_broadcast(&condc);
            pthread_mutex_unlock(&mutex);
            break;
        }
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    double end_time = get_time();

    if (final_result_nonce == 0) {
        printf("No solution found!\n");
        return 1;
    }

    /* When printed in hex, a SHA-1 checksum will be 40 characters. */
    char solution_hash[41];
    sha1tostring(solution_hash, final_result_digest);

    printf("Solution found by thread %d:\n", final_thread);
    printf("Nonce: %lu\n", final_result_nonce);
    printf(" Hash: %s\n", solution_hash);

    double total_time = end_time - start_time;
    printf("%llu hashes in %.2fs (%.2f hashes/sec)\n",
            total_inversions, total_time, total_inversions / total_time);

    pthread_mutex_destroy(&mutex);
    elist_destroy(task_list);
    for(int i = 0; i < num_threads; ++i){
        free(arg_thread[i]);
    }
    free(arg_thread);
    free(threads);
    return 0;
}
