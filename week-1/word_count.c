/*
 * word_count.c — Week 1, Problem 3
 *
 * Parallel word counter (BROKEN — no synchronization, on purpose).
 *
 * Splits a text buffer into chunks, one per thread. Each thread
 * counts whitespace-delimited words in its chunk and adds to a
 * shared total. The shared total will exhibit a race condition.
 *
 * The sequential count at the end lets you compare and see the error.
 *
 * Compile:
 *   gcc -Wall -pthread word_count.c -o word_count
 *
 * Run multiple times:
 *   for i in $(seq 1 10); do ./word_count; done
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#define NUM_THREADS 4

/* Build a large text buffer by repeating a sentence. */
#define SENTENCE "the quick brown fox jumps over the lazy dog "
#define REPEATS  200000

char  *text_buf  = NULL;
long   text_len  = 0;
long   total_words = 0;  /* SHARED — race condition lives here */

typedef struct {
    int  thread_id;
    long start;
    long end;       /* exclusive */
    long local_count;
} thread_arg_t;

/*
 * Count words in text_buf[start..end). A word starts whenever we
 * transition from whitespace (or start of range) to non-whitespace.
 */
void *count_words(void *arg) {
    thread_arg_t *t = (thread_arg_t *)arg;
    long count = 0;
    int in_word = 0;

    for (long i = t->start; i < t->end; i++) {
        if (isspace((unsigned char)text_buf[i])) {
            in_word = 0;
        } else {
            if (!in_word) {
                count++;
                in_word = 1;
            }
        }
    }

    t->local_count = count;

    /* Add to the shared total — this is the race */
    for (long i = 0; i < count; i++) {
        total_words++;
    }

    printf("Thread %d: found %ld words in range [%ld, %ld)\n",
           t->thread_id, count, t->start, t->end);
    return NULL;
}

/* Sequential count for verification. */
long count_sequential(const char *buf, long len) {
    long count = 0;
    int in_word = 0;
    for (long i = 0; i < len; i++) {
        if (isspace((unsigned char)buf[i])) {
            in_word = 0;
        } else {
            if (!in_word) { count++; in_word = 1; }
        }
    }
    return count;
}

int main(void) {
    /* Build the text buffer */
    long sentence_len = (long)strlen(SENTENCE);
    text_len = sentence_len * REPEATS;
    text_buf = malloc(text_len + 1);
    if (!text_buf) { perror("malloc"); return 1; }

    for (long i = 0; i < REPEATS; i++) {
        memcpy(text_buf + i * sentence_len, SENTENCE, sentence_len);
    }
    text_buf[text_len] = '\0';

    /* Sequential baseline */
    long seq_count = count_sequential(text_buf, text_len);
    printf("Sequential word count: %ld\n\n", seq_count);

    /* Parallel (racy) count */
    pthread_t threads[NUM_THREADS];
    thread_arg_t args[NUM_THREADS];
    long chunk = text_len / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].thread_id   = i;
        args[i].start       = i * chunk;
        args[i].end         = (i == NUM_THREADS - 1) ? text_len
                                                      : (i + 1) * chunk;
        args[i].local_count = 0;
        pthread_create(&threads[i], NULL, count_words, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    /* Sum the per-thread local counts (race-free) for comparison */
    long sum_local = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        sum_local += args[i].local_count;
    }

    printf("\n--- Results ---\n");
    printf("Sequential count:          %ld\n", seq_count);
    printf("Sum of local counts:       %ld\n", sum_local);
    printf("Shared total (racy):       %ld\n", total_words);
    printf("Lost updates:              %ld\n", sum_local - total_words);

    free(text_buf);
    return 0;
}
