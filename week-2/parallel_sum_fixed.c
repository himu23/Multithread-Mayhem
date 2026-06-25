/*
 * parallel_sum_fixed.c — Week 2, Problem 1a
 *
 * Fixed version of week-1/code/03_parallel_sum.c.
 *
 * Fix: Each thread accumulates into a LOCAL variable, then adds
 * that local sum to the shared total under a mutex lock.
 * This minimizes lock contention while eliminating the race.
 *
 * Compile:
 *   gcc -Wall -pthread parallel_sum_fixed.c -o parallel_sum_fixed
 *
 * Run:
 *   ./parallel_sum_fixed
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ARRAY_SIZE  10000000
#define NUM_THREADS 4

int *array;
long total = 0;

pthread_mutex_t total_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int  thread_id;
    long start_index;
    long end_index;
} thread_arg_t;

void *partial_sum(void *arg) {
    thread_arg_t *t = (thread_arg_t *)arg;

    /* Accumulate locally — no lock needed here. */
    long local_sum = 0;
    for (long i = t->start_index; i < t->end_index; i++) {
        local_sum += array[i];
    }

    /* One lock acquisition to merge the result. */
    pthread_mutex_lock(&total_lock);
    total += local_sum;
    pthread_mutex_unlock(&total_lock);

    printf("Thread %d: partial sum = %ld (range [%ld, %ld))\n",
           t->thread_id, local_sum, t->start_index, t->end_index);
    return NULL;
}

int main(void) {
    array = malloc(ARRAY_SIZE * sizeof(int));
    if (!array) { perror("malloc"); return 1; }

    for (long i = 0; i < ARRAY_SIZE; i++) {
        array[i] = 1;
    }

    pthread_t threads[NUM_THREADS];
    thread_arg_t args[NUM_THREADS];
    long chunk = ARRAY_SIZE / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].thread_id   = i;
        args[i].start_index = i * chunk;
        args[i].end_index   = (i == NUM_THREADS - 1) ? ARRAY_SIZE
                                                      : (i + 1) * chunk;
        pthread_create(&threads[i], NULL, partial_sum, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nComputed sum: %ld\n", total);
    printf("Expected:     %d\n",  ARRAY_SIZE);

    if (total == ARRAY_SIZE) {
        printf("CORRECT — race condition eliminated.\n");
    } else {
        printf("WRONG — something is still broken.\n");
    }

    pthread_mutex_destroy(&total_lock);
    free(array);
    return 0;
}
