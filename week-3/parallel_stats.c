/*
 * parallel_stats.c — Week 3, Problem 1
 *
 * Compute statistics (min, max, sum, average) over a large array
 * using multiple threads. Each thread returns its partial results
 * via a heap-allocated struct, which the main thread collects via
 * pthread_join.
 *
 * Demonstrates:
 *   - Passing data to threads via a struct
 *   - Returning data from threads safely (heap allocation)
 *   - Collecting results with pthread_join's second argument
 *
 * Compile:
 *   gcc -Wall -pthread -std=c11 parallel_stats.c -o parallel_stats
 *
 * Run:
 *   ./parallel_stats
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>

#define ARRAY_SIZE  10000000
#define NUM_THREADS 4

int *array;

typedef struct {
    int  thread_id;
    long start;
    long end;       /* exclusive */
} thread_arg_t;

typedef struct {
    long sum;
    int  min;
    int  max;
    long count;
} thread_result_t;

void *compute_stats(void *arg) {
    thread_arg_t *t = (thread_arg_t *)arg;

    /* Allocate result on the heap — returning a stack pointer is UB. */
    thread_result_t *res = malloc(sizeof(thread_result_t));
    if (!res) { perror("malloc"); pthread_exit(NULL); }

    res->sum   = 0;
    res->min   = INT_MAX;
    res->max   = INT_MIN;
    res->count = t->end - t->start;

    for (long i = t->start; i < t->end; i++) {
        int val = array[i];
        res->sum += val;
        if (val < res->min) res->min = val;
        if (val > res->max) res->max = val;
    }

    printf("Thread %d: sum=%ld, min=%d, max=%d, count=%ld\n",
           t->thread_id, res->sum, res->min, res->max, res->count);

    return res;  /* returned to pthread_join */
}

int main(void) {
    srand(42);

    /* Fill array with random values 0–999. */
    array = malloc(ARRAY_SIZE * sizeof(int));
    if (!array) { perror("malloc"); return 1; }
    for (long i = 0; i < ARRAY_SIZE; i++) {
        array[i] = rand() % 1000;
    }

    pthread_t threads[NUM_THREADS];
    thread_arg_t args[NUM_THREADS];
    long chunk = ARRAY_SIZE / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].thread_id = i;
        args[i].start     = i * chunk;
        args[i].end       = (i == NUM_THREADS - 1) ? ARRAY_SIZE
                                                    : (i + 1) * chunk;
        pthread_create(&threads[i], NULL, compute_stats, &args[i]);
    }

    /* Collect results via pthread_join. */
    long total_sum   = 0;
    int  global_min  = INT_MAX;
    int  global_max  = INT_MIN;
    long total_count = 0;

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_result_t *res;
        pthread_join(threads[i], (void **)&res);

        if (res) {
            total_sum   += res->sum;
            total_count += res->count;
            if (res->min < global_min) global_min = res->min;
            if (res->max > global_max) global_max = res->max;
            free(res);
        }
    }

    double average = (double)total_sum / total_count;

    printf("\n--- Global Statistics ---\n");
    printf("Count:   %ld\n", total_count);
    printf("Sum:     %ld\n", total_sum);
    printf("Min:     %d\n",  global_min);
    printf("Max:     %d\n",  global_max);
    printf("Average: %.2f\n", average);

    /* Sequential verification */
    long seq_sum = 0;
    int  seq_min = INT_MAX, seq_max = INT_MIN;
    for (long i = 0; i < ARRAY_SIZE; i++) {
        seq_sum += array[i];
        if (array[i] < seq_min) seq_min = array[i];
        if (array[i] > seq_max) seq_max = array[i];
    }

    printf("\n--- Sequential Verification ---\n");
    printf("Sum:     %ld  %s\n", seq_sum,
           seq_sum == total_sum ? "✓" : "✗ MISMATCH");
    printf("Min:     %d   %s\n", seq_min,
           seq_min == global_min ? "✓" : "✗ MISMATCH");
    printf("Max:     %d   %s\n", seq_max,
           seq_max == global_max ? "✓" : "✗ MISMATCH");

    free(array);
    return 0;
}
