/*
 * bank_chaos_fixed.c — Week 2, Problem 1b
 *
 * Fixed version of week-1/code/04_bank_chaos.c.
 *
 * Fix: Wrap the check-then-act (balance check + withdrawal) in a
 * mutex lock so the entire operation is atomic with respect to
 * other threads.
 *
 * Compile:
 *   gcc -Wall -pthread bank_chaos_fixed.c -o bank_chaos_fixed
 *
 * Run:
 *   ./bank_chaos_fixed
 */

#include <stdio.h>
#include <pthread.h>

int balance = 10000;

#define ATTEMPTS_PER_CUSTOMER 100000
#define WITHDRAWAL_AMOUNT     1

pthread_mutex_t balance_lock = PTHREAD_MUTEX_INITIALIZER;

void *customer(void *arg) {
    const char *name = (const char *)arg;
    int successful = 0;
    int rejected   = 0;

    for (int i = 0; i < ATTEMPTS_PER_CUSTOMER; i++) {
        pthread_mutex_lock(&balance_lock);

        /* Now check-then-act is inside the critical section.
         * No other thread can read or modify balance between
         * our check and our subtraction. */
        if (balance >= WITHDRAWAL_AMOUNT) {
            balance -= WITHDRAWAL_AMOUNT;
            successful++;
        } else {
            rejected++;
        }

        pthread_mutex_unlock(&balance_lock);
    }

    printf("Customer %s: %d successful, %d rejected\n",
           name, successful, rejected);
    return NULL;
}

int main(void) {
    pthread_t alice, bob;

    pthread_create(&alice, NULL, customer, (void *)"Alice");
    pthread_create(&bob,   NULL, customer, (void *)"Bob");

    pthread_join(alice, NULL);
    pthread_join(bob,   NULL);

    printf("Final balance: %d\n", balance);

    if (balance < 0) {
        printf("BUG: balance went negative. Fix didn't work.\n");
    } else if (balance == 0) {
        printf("Balance is exactly zero — correct!\n");
        printf("All $10000 withdrawn, none lost, none duplicated.\n");
    } else {
        printf("Balance is %d — some withdrawals were correctly rejected.\n",
               balance);
    }

    pthread_mutex_destroy(&balance_lock);
    return 0;
}
