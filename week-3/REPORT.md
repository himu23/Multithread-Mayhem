# Week 3 — Report

## Problems Attempted
Problems 1 and 2.

## Problem 1 — Parallel Statistics (parallel_stats.c)

Computed min, max, sum, and average over an array of 10,000,000 random values using 4 threads.

**Key design decisions:**
- Each thread receives its chunk boundaries via a `thread_arg_t` struct.
- Each thread returns a heap-allocated `thread_result_t` containing partial sum, min, max, and count.
- The main thread collects results via `pthread_join`'s second argument, merges them, then frees the memory.

**Results:**
- Parallel results matched sequential verification perfectly across 10+ runs.
- No race condition because threads never write to shared state — each works on its own chunk and returns a private result.

**Lesson learned:** Returning data from threads requires heap allocation. My first attempt returned a pointer to a local struct on the thread's stack — it worked 9 out of 10 times (the stack memory hadn't been overwritten yet), but the 10th run produced garbage values. Classic dangling pointer bug.

## Problem 2 — Semaphore Bounded Buffer (sem_buffer.c)

Re-implemented the Week 2 bounded buffer using POSIX semaphores instead of condition variables.

**How it differs from the Week 2 condvar version:**
1. **No while-loop around waits.** `sem_wait` blocks until the count is positive and decrements atomically. The count IS the condition — no need to re-check after waking up.
2. **Cleaner separation of concerns.** Semaphores handle the "is there room / is there data?" logic. The mutex only protects the buffer array and index variables.
3. **Slightly simpler code.** Two semaphores replace two condvars + the while-loop logic.

**Testing:** Ran with 2 producers (15 items each) and 2 consumers (15 items each) = 30 total items. All items consumed exactly once in every run.

## What Surprised Me
The semaphore version is noticeably cleaner than the condvar version. The condvar approach requires you to think about "what condition am I waiting for?" and "what if I wake up and the condition is no longer true?" (spurious wakeups). With semaphores, the count tracks the condition directly — `sem_wait` only returns when it can make progress. The mental model is simpler.

## Where I Got Stuck
I initially put `sem_post` before `pthread_mutex_unlock` in the producer. This caused the consumer to wake up and immediately contend for the lock while the producer still held it. Moving `sem_post` after `pthread_mutex_unlock` eliminated the unnecessary contention. The theory notes mentioned this as a best practice — signal after unlocking.
