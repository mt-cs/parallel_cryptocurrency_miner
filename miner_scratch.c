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

#include "sha1.h"
#include "logger.h"
#define MAX_INPUT_LENGTH (10)

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

unsigned long long total_inversions; // everytime we do an inversion counter how many hashes computed per second

struct thread_struct {
    uint32_t difficulty_mask;
    char *bitcoin_block_data;
    pthread_t thread_id;
} t_struct;

double get_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
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

int get_strtol(char *num)
{
    char *rest;
    long threads = strtol(num, &rest, MAX_INPUT_LENGTH);
    return (int) threads;
}


int get_bitcount(unsigned char x) 
{ 
   int count;
   
   for (count=0; x != 0; x>>=1)
   {
      if ( x & 01)
         count++;
   }

   return count;
}

void *consumer_thread(void *ptr) {
//     unsigned long long i;

//     unsigned long rank = (unsigned long) ptr;
//     unsigned long long start = block_size * rank; // TODO: block size to Bitcoin block
//     unsigned long long end = block_size * rank + total_inversions;
// #ifdef DEBUG
//     fprintf(stderr, "I am thread %ld calculating range %Ld to %Ld\n",
//             (long) rank, start, end);
// #endif

//     for (i = start; i <= end; ++i) {
//         pthread_mutex_lock(&mutex);

//         // in the real world we want to do the heavy work here, example: hash function
    

//         /* Wake up the producer */
//         pthread_cond_signal(&cond); // send signal
//         pthread_mutex_unlock(&mutex); // unlock

        //sleep(1); // go to sleep, we can turn it off to make it faster
    }

    return 0;
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Usage: %s threads difficulty 'block data (string)'\n", argv[0]);
        return EXIT_FAILURE;
    }


    // calculate bitcoin block!!!
    
    //int num_threads = 1; // TODO argv[1]
    char *str_threads = argv[1];
    int num_threads = get_strtol(str_threads);
    printf("Number of threads: %d\n", num_threads);
    
    //struct t_struct = malloc(sizeof(struct node));


    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], NULL, consumer_thread, (void *)&t_struct);
    }

    // TODO we have hard coded the difficulty to 20 bits (0x0000FFF). This is a
    // fairly quick computation -- something like 28 will take much longer.  You
    // should allow the user to specify anywhere between 1 and 32 bits of zeros.

    
    // for loop start with in that start to 0, set the bit to 1 or 0 depending on user input
    // bit manipulation bit wise or . set individual bit to 1 or 0 
    uint32_t difficulty_mask = 0x00000FFF; // TODO: check global
    
    //difficulty_mask = 0x00000FFF;
    printf("  Difficulty Mask: "); //argv[2]
    char *diff_str = argv[2];
    LOG("Difficulty mask string: %s\n", diff_str);
    //uint32_t difficulty_mask = argv[2];
    LOG("%d\n", difficulty_mask);
    print_binary32(difficulty_mask);
    

    /* We use the input string passed in (argv[3]) as our block data. In a
     * complete bitcoin miner implementation, the block data would be composed
     * of bitcoin transactions. */
    // char *bitcoin_block_data = argv[3]; // TODO: check global

    bitcoin_block_data = argv[3];
    printf("       Block data: [%s]\n", bitcoin_block_data);

    printf("\n----------- Starting up miner threads!  -----------\n\n");
    // consumer put it in the thread procedure and call the mine function
    // start with single producer and consumer and see if it works!!!!!

    double start_time = get_time();

    uint8_t digest[SHA1_HASH_SIZE]; // TODO: ASK wth is this????

    // main thread producer
    // create one worker thread to start out with
    // refactor the prod cons to make it work

    /* Mine the block. */ // can't call pthread routine because it only takes one parameter -----------------------> go to consumer
    uint64_t nonce = mine(
            bitcoin_block_data,
            difficulty_mask,
            1, UINT64_MAX, // range of nonce
            digest);

    // worker thread split the range of the nonce
    // have each thread do 100 nonces
    // on the producer (main thread) create a new task to work on, e.g. 1 - 100
    // worker thread 0 found task, signal it, pick the task and work on it
    // main thread keeps repeatingly creating task for the workers
    // rather than one thead start from the middle like pithread
    // workers keep grabbing task from main thread and working silmuntaneosly
    // e.g. worker now create 101 - 200
    // worker thread 1 found task, signal it, pick the task and work on it
    
    double end_time = get_time();
    
    if (nonce == 0) {
        printf("No solution found!\n");
        return 1;
    }

    /* When printed in hex, a SHA-1 checksum will be 40 characters. */
    char solution_hash[41];
    sha1tostring(solution_hash, digest);

    printf("Solution found by thread %d:\n", 0);
    printf("Nonce: %lu\n", nonce);
    printf(" Hash: %s\n", solution_hash);

    double total_time = end_time - start_time;
    printf("%llu hashes in %.2fs (%.2f hashes/sec)\n",
            total_inversions, total_time, total_inversions / total_time);

    return 0;
}
