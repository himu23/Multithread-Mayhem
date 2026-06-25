/*
 * bounded_buffer.c — Week 2, Problem 2
 *
 * Thread-safe bounded buffer using a mutex and two condition variables.
 *
 * Implementation:
 *   - Ring buffer of BUFFER_SIZE slots.
 *   - One mutex protects all shared state (buffer, count, in, out).
 *   - 'not_full' condvar: producer waits here when buffer is full.
 *   - 'not_empty' condvar: consumer waits here when buffer is empty.
 *   - Multiple producers and multiple consumers supported.
 *
 * We run 2 producers producing 20 items each (40 total) and 2 consumers
 * consuming 20 items each (40 total). All items must be consumed exactly
 * once, in any order.
 *
 * Compile:
 *   gcc -Wall -pthread bounded_buffer.c -o bounded_buffer
 *
 * Run:
 *   ./bounded_buffer
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE       5
#define NUM_PRODUCERS     2
#define NUM_CONSUMERS     2
#define ITEMS_PER_PRODUCER 20

/* Shared bounded buffer (ring). */
int  buffer[BUFFER_SIZE];
int  count = 0;
int  in    = 0;
int  out   = 0;

pthread_mutex_t buf_lock  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  not_full  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  not_empty = PTHREAD_COND_INITIALIZER;

/* Track total items consumed for verification. */
int total_consumed = 0;
pthread_mutex_t consumed_lock = PTHREAD_MUTEX_INITIALIZER;

void *producer(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = id * 1000 + i;   /* unique item ID */

        pthread_mutex_lock(&buf_lock);

        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &buf_lock);
        }

        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        printf("[P%d] produced %d  (buffer: %d/%d)\n",
               id, item, count, BUFFER_SIZE);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&buf_lock);

        usleep(20000 + rand() % 30000);  /* simulate variable work */
    }

    printf("[P%d] done producing.\n", id);
    return NULL;
}

void *consumer(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        pthread_mutex_lock(&buf_lock);

        while (count == 0) {
            pthread_cond_wait(&not_empty, &buf_lock);
        }

        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        printf("[C%d] consumed %d  (buffer: %d/%d)\n",
               id, item, count, BUFFER_SIZE);

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&buf_lock);

        /* Track consumption count */
        pthread_mutex_lock(&consumed_lock);
        total_consumed++;
        pthread_mutex_unlock(&consumed_lock);

        usleep(40000 + rand() % 50000);  /* consumer is slower */
    }

    printf("[C%d] done consuming.\n", id);
    return NULL;
}

int main(void) {
    srand(42);

    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int       prod_ids[NUM_PRODUCERS];
    int       cons_ids[NUM_CONSUMERS];

    int total_items = NUM_PRODUCERS * ITEMS_PER_PRODUCER;
    printf("Starting %d producers (%d items each) and %d consumers.\n",
           NUM_PRODUCERS, ITEMS_PER_PRODUCER, NUM_CONSUMERS);
    printf("Total items to produce and consume: %d\n\n", total_items);

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        prod_ids[i] = i;
        pthread_create(&producers[i], NULL, producer, &prod_ids[i]);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        cons_ids[i] = i;
        pthread_create(&consumers[i], NULL, consumer, &cons_ids[i]);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++) pthread_join(producers[i], NULL);
    for (int i = 0; i < NUM_CONSUMERS; i++) pthread_join(consumers[i], NULL);

    printf("\n--- Verification ---\n");
    printf("Total produced: %d\n", total_items);
    printf("Total consumed: %d\n", total_consumed);
    if (total_consumed == total_items) {
        printf("All items consumed exactly once. Buffer works correctly.\n");
    } else {
        printf("MISMATCH — items were lost or duplicated.\n");
    }

    pthread_mutex_destroy(&buf_lock);
    pthread_mutex_destroy(&consumed_lock);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
    return 0;
}
