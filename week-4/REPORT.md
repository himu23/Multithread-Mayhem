# Week 4 — Report

## Problems Attempted
All four problems (1, 2, 3, 4).

## Problem 1 — Hello xv6! (hello_xv6.c)
Wrote a simple user program that prints a greeting, argument count, each argument, and the number of system calls in xv6 (21, counted manually from `kernel/syscall.h`).

Key observations:
- xv6 programs can only use `kernel/types.h` and `user/user.h` — no standard library headers.
- `exit(0)` is mandatory at the end. Without it, the program falls off `main` and causes a trap/panic.
- `argc` includes the program name itself, so actual user arguments = `argc - 1`.

## Problem 2 — Codebase Detective (codebase_detective.md)
Explored four areas of the xv6 source code. See `codebase_detective.md` for detailed answers.

Most interesting finding: the `scheduler()` function in `kernel/proc.c` is a dead-simple round-robin loop — scan the process table, find a RUNNABLE process, context-switch to it. No priority levels, no fancy scheduling algorithms. Yet it works well enough to run all of xv6's user programs smoothly. Simplicity wins for a teaching OS.

## Problem 3 — Fork Chain (fork_chain.c)
Created a chain of N processes using a loop with `fork()`. The parent at each depth prints its info, forks a child, then waits. Only the leaf (depth N-1) prints the leaf message.

Output matched expectations: PIDs were sequential (e.g., 4, 5, 6 for a chain of 3), and parent PIDs correctly pointed to the previous process in the chain. The `wait(0)` calls ensured clean bottom-up teardown — the leaf exits first, then depth N-2, etc.

**Twist verification:** Total processes = N, depth reached = N-1. Both matched the command-line argument.

## Problem 4 — Syscall Spy (syscall_spy.c)
Called 7 different system calls and printed their return values:
- `getpid()` → consistent PID across two calls
- `uptime()` → ticks since boot (varied per run)
- `fork()` → child PID in parent, 0 in child
- `wait()` → reaped PID matched fork's return value
- `sleep(10)` → returned 0
- `write(1, ...)` → returned 14 (bytes written)
- `dup(0)` → returned next available fd

The fork/wait PID consistency check passed every run.

## Most Interesting Source Code Finding
The trap handling path (`kernel/trap.c`, `trampoline.S`) was fascinating. When a user program makes a system call, the `ecall` instruction raises privilege to S-mode and jumps to `uservec` in `trampoline.S`. The trampoline page is mapped at the same virtual address in both user and kernel page tables — that's how the transition works without a page fault. After saving all registers to the trapframe, it switches to the kernel page table and calls `usertrap()` in C. The entire round-trip is ~20 lines of assembly. Elegant.

## What Surprised Me
That xv6 has no `malloc` in user space. `sbrk(n)` is the only way to allocate memory, and it just grows the process's address space. There's no `free`. Programs that need dynamic memory have to manage it themselves or just allocate and never free (acceptable for short-lived xv6 programs).

## Where I Got Stuck
Setting up the RISC-V cross-compiler on my machine. The `gcc-riscv64-unknown-elf` package wasn't available on my distro's default repos, so I used the Docker approach instead (`arghyadipchak/xv6-riscv-devbox`). Once that was set up, building and booting xv6 was straightforward.
