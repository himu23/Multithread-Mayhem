# Week 1 — Report

## Problems Attempted
Problems 1, 2, and 3.

## Problem 1 — Run, Observe, Write Down

### 01_hello_thread.c
- Ran 10+ times. Output was consistent: both threads always printed their messages, but the order varied across runs. Sometimes Thread 1 printed first, sometimes Thread 2 did. Occasionally their output interleaved mid-line. The join calls ensured both always completed before main exited.

### 02_race_counter.c
- Ran 15 times with ITERS = 10,000,000. Expected final counter: 20,000,000. Actual results ranged from ~14,000,000 to ~19,800,000. Only 1 out of 15 runs showed the correct value. The error was non-deterministic — sometimes off by millions, sometimes by thousands.

### 03_parallel_sum.c
- Ran 10 times with 4 threads. Expected sum: 10,000,000. Most runs were off by 50,000–200,000. More threads made the error worse. With NUM_THREADS = 8, the error was consistently larger.

### 04_bank_chaos.c
- Ran 10 times. The balance went negative in 7 out of 10 runs. The most negative value I saw was -14. The total successful withdrawals by Alice + Bob often exceeded 10,000, confirming the check-then-act race.

## Problem 2 — Make the Race Worse (race_amplified.c)
Three changes to amplify the race:
1. Increased to 8 threads (from 2) — more contention.
2. Separated the load-increment-store into three explicit steps with a `sched_yield()` between the load and the store. This forces context switches right in the critical window.
3. Used `volatile` on the counter to prevent the compiler from keeping it in a register, ensuring real memory traffic.

Result: with 8 threads × 1,000,000 iterations, the expected total is 8,000,000. The actual result was typically under 1,500,000 — over 80% of updates lost.

## Problem 3 — Parallel Word Count (word_count.c)
Built a large text buffer (repeating "the quick brown fox..." 200,000 times). Split it across 4 threads. Each thread counted words in its chunk and added to a shared `total_words` counter without locking.

- Sequential count: 1,800,000 words.
- Per-thread local counts summed correctly to the same number.
- Shared `total_words` was consistently lower (lost ~50,000–150,000 updates).
- Demonstrates that the race condition from the simple counter applies to any shared accumulation, not just toy examples.

## What Surprised Me
The bank_chaos example surprised me the most. The code *looks* correct — there's a balance check before every withdrawal. But the check and the subtraction are not atomic, so the check is meaningless under concurrency. It's a perfect example of how reading code line-by-line can fool you into thinking it's safe.

## Where I Got Stuck
Initially I forgot the `-pthread` flag and got linker errors. After re-reading the theory notes, I realized it's needed to link the pthreads library. Also, on my first few runs of race_counter I happened to get the "correct" answer, which made me think there was no bug — until I ran it 10+ times and saw the error.
