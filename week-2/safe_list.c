/*
 * safe_list.c — Week 2, Problem 3
 *
 * Thread-safe singly-linked list with coarse-grained locking.
 *
 * A single mutex protects all list operations. This is simple and
 * correct, though it limits concurrency (no two operations can
 * proceed simultaneously).
 *
 * Operations:
 *   - insert(value): insert at head
 *   - search(value): return 1 if found, 0 otherwise
 *   - delete(value): remove first occurrence, return 1 if found
 *   - print_list(): print all elements
 *
 * Compile:
 *   gcc -Wall -pthread safe_list.c -o safe_list
 *
 * Run:
 *   ./safe_list
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/* ---- Linked list ------------------------------------------------- */

typedef struct node {
    int value;
    struct node *next;
} node_t;

node_t *head = NULL;
pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;

void list_insert(int value) {
    node_t *new = malloc(sizeof(node_t));
    if (!new) { perror("malloc"); exit(1); }
    new->value = value;

    pthread_mutex_lock(&list_lock);
    new->next = head;
    head = new;
    pthread_mutex_unlock(&list_lock);
}

int list_search(int value) {
    pthread_mutex_lock(&list_lock);
    node_t *cur = head;
    while (cur) {
        if (cur->value == value) {
            pthread_mutex_unlock(&list_lock);
            return 1;
        }
        cur = cur->next;
    }
    pthread_mutex_unlock(&list_lock);
    return 0;
}

int list_delete(int value) {
    pthread_mutex_lock(&list_lock);
    node_t **pp = &head;
    while (*pp) {
        if ((*pp)->value == value) {
            node_t *victim = *pp;
            *pp = victim->next;
            free(victim);
            pthread_mutex_unlock(&list_lock);
            return 1;
        }
        pp = &(*pp)->next;
    }
    pthread_mutex_unlock(&list_lock);
    return 0;
}

void list_print(void) {
    pthread_mutex_lock(&list_lock);
    printf("List: ");
    node_t *cur = head;
    while (cur) {
        printf("%d -> ", cur->value);
        cur = cur->next;
    }
    printf("NULL\n");
    pthread_mutex_unlock(&list_lock);
}

/* ---- Test threads ------------------------------------------------ */

void *inserter(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < 10; i++) {
        int val = id * 100 + i;
        list_insert(val);
        printf("[Inserter %d] inserted %d\n", id, val);
        usleep(10000);
    }
    return NULL;
}

void *searcher(void *arg) {
    (void)arg;
    for (int i = 0; i < 15; i++) {
        int target = rand() % 200;
        int found  = list_search(target);
        printf("[Searcher] %d %s\n", target,
               found ? "FOUND" : "not found");
        usleep(15000);
    }
    return NULL;
}

void *deleter(void *arg) {
    (void)arg;
    usleep(50000);  /* let some inserts happen first */
    for (int i = 0; i < 8; i++) {
        int target = rand() % 200;
        int deleted = list_delete(target);
        printf("[Deleter] %d %s\n", target,
               deleted ? "DELETED" : "not found");
        usleep(20000);
    }
    return NULL;
}

int main(void) {
    srand(42);

    pthread_t t_ins[2], t_search, t_delete;
    int ids[] = {0, 1};

    pthread_create(&t_ins[0],  NULL, inserter, &ids[0]);
    pthread_create(&t_ins[1],  NULL, inserter, &ids[1]);
    pthread_create(&t_search,  NULL, searcher, NULL);
    pthread_create(&t_delete,  NULL, deleter,  NULL);

    pthread_join(t_ins[0], NULL);
    pthread_join(t_ins[1], NULL);
    pthread_join(t_search, NULL);
    pthread_join(t_delete, NULL);

    printf("\n--- Final state ---\n");
    list_print();

    /* Cleanup */
    while (head) {
        node_t *tmp = head;
        head = head->next;
        free(tmp);
    }

    pthread_mutex_destroy(&list_lock);
    return 0;
}
