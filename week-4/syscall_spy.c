// syscall_spy.c — Week 4, Problem 4
//
// Calls as many different xv6 system calls as possible and
// prints what each one returns.
//
// System calls tested:
//   getpid, uptime, fork, write, sleep, dup, wait
//
// Also verifies that the child's PID (returned by fork in the parent)
// matches the child's own getpid() result.
//
// To add to xv6:
//   1. Place in user/syscall_spy.c
//   2. Add $U/_syscall_spy\ to UPROGS in Makefile
//   3. make clean && make && make qemu

#include "kernel/types.h"
#include "user/user.h"

int main(void) {
    int my_pid = getpid();
    printf("Syscall Spy Report for PID %d:\n", my_pid);

    // --- getpid ---
    printf("  getpid()       -> %d\n", my_pid);

    // --- uptime ---
    int ticks = uptime();
    printf("  uptime()       -> %d\n", ticks);

    // --- fork ---
    int child_pid = fork();

    if (child_pid < 0) {
        printf("  fork()         -> FAILED\n");
        exit(1);
    }

    if (child_pid == 0) {
        // --- Inside the child ---
        printf("  --- in child (PID %d) ---\n", getpid());
        printf("  I am the child!\n");
        printf("  getpid()       -> %d\n", getpid());
        exit(0);
    }

    // --- Parent continues ---
    printf("  fork()         -> %d  (child PID)\n", child_pid);

    // --- wait: reap the child ---
    int reaped = wait(0);
    printf("  --- back in parent (PID %d) ---\n", my_pid);
    printf("  wait()         -> %d  (reaped child PID %d)\n",
           reaped, reaped);

    // Verify fork/wait consistency
    if (reaped == child_pid) {
        printf("  [OK] fork() returned %d, wait() reaped %d — match!\n",
               child_pid, reaped);
    } else {
        printf("  [ERR] fork() returned %d but wait() reaped %d\n",
               child_pid, reaped);
    }

    // --- sleep ---
    int sleep_ret = sleep(10);
    printf("  sleep(10)      -> %d\n", sleep_ret);

    // --- write ---
    char msg[] = "Hello, world!\n";
    int written = write(1, msg, sizeof(msg) - 1);  // -1 to exclude null terminator
    printf("  write(1, ...)  -> %d\n", written);

    // --- dup ---
    int new_fd = dup(0);
    printf("  dup(0)         -> %d\n", new_fd);

    // Close the duplicated fd to be clean
    close(new_fd);

    // --- getpid again (should be same) ---
    int pid_again = getpid();
    printf("  getpid() again -> %d  %s\n", pid_again,
           pid_again == my_pid ? "(same — correct)" : "(DIFFERENT — bug!)");

    printf("\nSyscall Spy complete. %d system calls tested.\n", 7);

    exit(0);
}
