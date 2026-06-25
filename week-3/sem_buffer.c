/*
 * sem_buffer.c — Week 3, Problem 2
 *
 * Bounded buffer using POSIX semaphores instead of condition variables.
 *
 * Two semaphores replace the two condvars from Week 2:
 *   'empty_slots' = number of free slots (initialized to BUFFER_SIZE)
 *   'full_slots'  = number of items ready (initialized to 0)
 *
 * Producer: sem_wait(empty_slots) → lock → enqueue → unlock → sem_post(full_slots)
 * Consumer: sem_wait(full_slots)  → lock → dequeue → unlock → sem_post(empty_slots)
 *
 * Key difference from the condvar version:
 *   - No while-loop around the wait. The semaphore count IS the condition.
 *   - Cleaner separation of concerns: semaphores handle the "is there room / data?"
 *     question, the mutex handles "is the buffer being modified?".
 *
 * Compile:
 *   gcc -Wall -pthread -std=c11 sem_buffer.c -o sem_buffer
 *
 * Run:
 *   ./sem_buffer
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE         5
#define ITEMS_PER_PRODUCER  15
#define NUM_PRODUCERS       2
#define NUM_CONSUMERS       2

/* Shared ring buffer. */
int buffer[BUFFER_SIZE];
int in = 0, out = 0;

/* Semaphores for slot management. */
sem_t empty_slots;   /* counts free slots */
sem_t full_slots;    /* counts available items */

/* Mutex protects buffer internals (in, out, buffer[]). */
pthread_mutex_t buf_lock = PTHREAD_MUTEX_INITIALIZER;

/* For verification. */
int total_consumed = 0;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;

void *producer(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = id * 1000 + i;

        sem_wait(&empty_slots);            /* wait for a free slot */
        pthread_mutex_lock(&buf_lock);

        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;

        int items_ready;
        sem_getvalue(&full_slots, &items_ready);
        printf("[P%d] produced %d  (items after: %d)\n",
               id, item, items_ready + 1);

        pthread_mutex_unlock(&buf_lock);
        sem_post(&full_slots);             /* signal: new item available */

        usleep(30000 + rand() % 20000);
    }

    printf("[P%d] finished.\n", id);
    return NULL;
}

void *consumer(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        sem_wait(&full_slots);             /* wait for an item */
        pthread_mutex_lock(&buf_lock);

        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;

        int slots_free;
        sem_getvalue(&empty_slots, &slots_free);
        printf("[C%d] consumed %d  (free slots after: %d)\n",
               id, item, slots_free + 1);

        pthread_mutex_unlock(&buf_lock);
        sem_post(&empty_slots);            /* signal: slot freed */

        pthread_mutex_lock(&count_lock);
        total_consumed++;
        pthread_mutex_unlock(&count_lock);

        usleep(60000 + rand() % 40000);
    }

    printf("[C%d] finished.\n", id);
    return NULL;
}

int main(void) {
    srand(42);

    sem_init(&empty_slots, 0, BUFFER_SIZE);  /* all slots free */
    sem_init(&full_slots,  0, 0);            /* no items yet */

    int total_items = NUM_PRODUCERS * ITEMS_PER_PRODUCER;
    printf("=== Semaphore Bounded Buffer ===\n");
    printf("Buffer size: %d, Producers: %d, Consumers: %d\n",
           BUFFER_SIZE, NUM_PRODUCERS, NUM_CONSUMERS);
    printf("Total items: %d\n\n", total_items);

    pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMERS];
    int       prod_ids[NUM_PRODUCERS],  cons_ids[NUM_CONSUMERS];

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
    printf("Produced: %d, Consumed: %d\n", total_items, total_consumed);
    if (total_consumed == total_items)
        printf("All items consumed correctly. No data lost.\n");
    else
        printf("ERROR: mismatch detected.\n");

    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    pthread_mutex_destroy(&buf_lock);
    pthread_mutex_destroy(&count_lock);
    return 0;
}
