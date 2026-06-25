/*
 * race_amplified.c — Week 1, Problem 2
 *
 * Goal: make the race condition from 02_race_counter.c worse.
 *
 * Changes from the original:
 *   1. Increased from 2 threads to 8 — more contention on the counter.
 *   2. Added a dummy computation inside the loop body to make the
 *      critical section longer (more time for interleaving).
 *   3. Used volatile to prevent the compiler from optimizing away
 *      the shared counter (ensures real memory loads/stores).
 *   4. Added sched_yield() after every increment to voluntarily give
 *      up the CPU, maximizing the chance of interleaving right at
 *      the dangerous moment between load and store.
 *
 * Compile:
 *   gcc -Wall -pthread race_amplified.c -o race_amplified
 *
 * Run multiple times:
 *   for i in $(seq 1 10); do ./race_amplified; done
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>

#define NUM_THREADS 8
#define ITERS       1000000

/*
 * volatile prevents the compiler from keeping 'counter' in a register.
 * Every read/write must go to memory, which makes the race window
 * much wider.
 */
volatile long counter = 0;

void *increment(void *arg) {
    int id = *(int *)arg;
    long local_count = 0;

    for (long i = 0; i < ITERS; i++) {
        /*
         * Intentionally read, compute, then write — three separate
         * steps — to widen the race window.
         */
        long temp = counter;
        temp = temp + 1;

        /* Yield the CPU between read and write. This is the key trick:
         * another thread is very likely to run its own read-modify-write
         * in this gap, clobbering our value. */
        sched_yield();

        counter = temp;
        local_count++;
    }

    printf("Thread %d finished, performed %ld increments\n", id, local_count);
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    int       ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, increment, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    long expected = (long)NUM_THREADS * ITERS;
    long lost     = expected - counter;

    printf("\n--- Results ---\n");
    printf("Final counter:  %ld\n", counter);
    printf("Expected:       %ld\n", expected);
    printf("Lost updates:   %ld  (%.1f%%)\n", lost,
           100.0 * lost / expected);

    return 0;
}
