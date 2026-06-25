# Week 4, Problem 2 — Codebase Detective

## A. Process States (`kernel/proc.h`)

**What are the five possible states of an xv6 process?**
```c
enum procstate { UNUSED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
```
1. `UNUSED` — slot in the process table is free
2. `SLEEPING` — waiting for an event (e.g., I/O, child exit)
3. `RUNNABLE` — ready to run, waiting for CPU time
4. `RUNNING` — currently executing on a CPU
5. `ZOMBIE` — has exited but parent hasn't called `wait()` yet

**What state is a process in right after `fork()` returns in the child?**
`RUNNABLE` — the child is marked runnable by `allocproc()` + `fork()` so the scheduler can pick it up. It hasn't been selected to run yet (that's RUNNING), but it's ready to be scheduled.

**What state is a process in between `exit()` and `wait()`?**
`ZOMBIE` — when a process calls `exit()`, it sets its state to ZOMBIE. The process's memory and page table are freed, but its `struct proc` entry remains in the table (holding the exit status) until the parent calls `wait()` and reaps it.

---

## B. The Boot Process (`kernel/main.c`)

**Initialization steps executed in `main()`, in order:**
1. `consoleinit()` — initialize the UART console driver
2. `printfinit()` — initialize the kernel printf lock
3. `kinit()` — initialize the physical memory allocator (free pages)
4. `kvminit()` — create the kernel page table
5. `kvminithart()` — install the kernel page table on this CPU
6. `procinit()` — initialize the process table and per-process kernel stacks
7. `trapinit()` — set up the trap vector table
8. `trapinithart()` — install the trap vector on this CPU
9. `plicinit()` — set up the PLIC (Platform-Level Interrupt Controller)
10. `plicinithart()` — configure PLIC for this CPU's interrupt priorities
11. `binit()` — initialize the buffer cache (block I/O layer)
12. `iinit()` — initialize the inode cache
13. `fileinit()` — initialize the file table
14. `virtio_disk_init()` — initialize the virtio disk driver
15. `userinit()` — create the first user process (`init`)
16. `scheduler()` — start the scheduler (never returns)

**First subsystem initialized:** `consoleinit()` — the console, so the kernel can print messages.
**Last subsystem initialized:** `userinit()` creates the first user process, then `scheduler()` is called (which never returns — it's the main loop).

**What does `scheduler()` do?** (from `kernel/proc.c`)
It runs an infinite loop: for each CPU, it scans the process table looking for a process in RUNNABLE state. When it finds one, it switches to RUNNING, performs a context switch to that process's saved registers, and lets it run. When the process yields (timer interrupt, sleep, exit), control returns to the scheduler, which picks the next RUNNABLE process. This is a simple round-robin scheduler — no priorities, no fairness guarantees.

---

## C. The System Call Table (`kernel/syscall.c`)

**How many system calls does xv6 support?**
21 system calls (SYS_fork through SYS_close, numbered 1–21).

**The `syscalls[]` array pattern:**
```c
static uint64 (*syscalls[])(void) = {
    [SYS_fork]    sys_fork,
    [SYS_exit]    sys_exit,
    ...
    [SYS_close]   sys_close,
};
```
The array is indexed by the system call number from `syscall.h`. The `[SYS_fork]` syntax is C designated initializers — each entry is placed at exactly the index matching its syscall number. This means `syscalls[1]` = `sys_fork`, `syscalls[2]` = `sys_exit`, etc. The indices and the `#define` values in `syscall.h` correspond one-to-one.

**What happens with an invalid system call number?**
In `syscall()` in `kernel/syscall.c`, there's a bounds check:
```c
if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    p->trapframe->a0 = syscalls[num]();
} else {
    printf("%d %s: unknown sys call %d\n", p->pid, p->name, num);
    p->trapframe->a0 = -1;
}
```
It prints an error message with the process PID, name, and the invalid syscall number, then returns -1 to the calling process.

---

## D. The Shell (`user/sh.c`)

**How does the shell run a command?**
In `runcmd()`, the shell uses a fork-exec pattern:
1. For `EXEC` commands: it calls `exec(ecmd->argv[0], ecmd->argv)` directly (note: `runcmd` is already called from a forked child).
2. The parent (in `main`'s loop) calls `fork()`, then the child calls `runcmd()`, and the parent calls `wait(0)` to block until the child finishes.

**What happens when you type `ls | cat`?**
The shell parses this as a `PIPE` command. In `runcmd()`, the `PIPE` case:
1. Calls `pipe(p)` to create a pipe (file descriptors `p[0]` for read, `p[1]` for write).
2. Forks a child for the left side (`ls`):
   - Closes stdout (fd 1), dups `p[1]` to fd 1, closes both pipe fds.
   - Calls `runcmd(pcmd->left)` which execs `ls` — its stdout now goes into the pipe.
3. Forks a child for the right side (`cat`):
   - Closes stdin (fd 0), dups `p[0]` to fd 0, closes both pipe fds.
   - Calls `runcmd(pcmd->right)` which execs `cat` — its stdin reads from the pipe.
4. Parent closes both pipe fds and calls `wait(0)` twice.

So `ls` writes to the pipe, `cat` reads from it. The data flows: `ls` stdout → pipe → `cat` stdin.
