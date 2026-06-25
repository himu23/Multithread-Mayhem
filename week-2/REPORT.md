# Week 2 — Report

## Problems Attempted
Problems 1, 2, and 3.

## Problem 1 — Fix Last Week's Mess

### parallel_sum_fixed.c
The original had every thread adding directly to the shared `total` inside a tight loop — a race on every single addition. The fix uses **local accumulation**: each thread sums its chunk into a local variable, then acquires the mutex once to merge the result.

- Correctness: Ran 20 times, always got 10,000,000. Race eliminated.
- Performance: The fixed version is actually *faster* than the broken original. The original had heavy contention on the shared `total` (cache bouncing between cores on every iteration). The fixed version does one lock acquisition per thread total, so threads run almost independently.

### bank_chaos_fixed.c
The original check-then-act pattern (`if (balance >= amount) { balance -= amount; }`) was not atomic. The fix wraps both the check and the act inside `pthread_mutex_lock / pthread_mutex_unlock`.

- Correctness: Ran 20 times. Balance never went negative. Alice + Bob's total successful withdrawals always summed to exactly 10,000.
- Performance: Slightly slower due to lock contention on every withdrawal attempt, but correctness is more important here.

## Problem 2 — Thread-Safe Bounded Buffer (bounded_buffer.c)
Implemented a ring buffer with:
- A single mutex (`buf_lock`) protecting buffer state (count, in, out).
- Two condition variables: `not_full` (producer waits when buffer is full) and `not_empty` (consumer waits when buffer is empty).
- Used `while` loops (not `if`) around `pthread_cond_wait` to handle spurious wakeups.
- Tested with 2 producers (20 items each) and 2 consumers (20 items each = 40 total).

Edge cases considered:
- Buffer starts empty: consumers block correctly on `not_empty`.
- Buffer fills up: producers block on `not_full` until consumers drain slots.
- Multiple producers/consumers: broadcast vs signal — used `pthread_cond_signal` (sufficient since each signal wakes exactly one waiter that will make progress).
- Verification: tracked total items consumed and confirmed it matches total produced.

## Problem 3 — Thread-Safe Linked List (safe_list.c)
Implemented a singly-linked list with coarse-grained locking (one mutex for the entire list). Supported operations: insert at head, search, delete, and print.

- Tested with 4 threads: 2 inserting, 1 searching, 1 deleting — all running concurrently.
- No crashes or lost nodes across 50+ test runs.

## What Surprised Me
The performance difference between the broken and fixed parallel_sum was counterintuitive. I expected the mutex version to be slower because of locking overhead. But the original version was doing millions of contested writes to a single cache line, which caused constant cache invalidation across cores. The fixed version avoids that entirely. Locks can make things *faster* when the alternative is cache thrashing.

## Where I Got Stuck
My first bounded buffer implementation used `if` instead of `while` around `pthread_cond_wait`. It worked most of the time but occasionally a consumer would read garbage — a spurious wakeup had allowed it to proceed when the buffer was actually still empty. Switching to `while` fixed it. The theory notes warned about this, but I had to see it myself to believe it.
