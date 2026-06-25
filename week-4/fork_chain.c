// fork_chain.c — Week 4, Problem 3
//
// Creates a chain of N processes, each forking the next.
//
// Usage: fork_chain [N]
//   N defaults to 5 if not specified.
//
// Each process prints its PID, parent PID, and depth.
// Only the leaf prints "I am the leaf!"
// Each process waits for its child before exiting.
//
// To add to xv6:
//   1. Place in user/fork_chain.c
//   2. Add $U/_fork_chain\ to UPROGS in Makefile
//   3. make clean && make && make qemu

#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int n = 5;  // default chain length

    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) {
            printf("Usage: fork_chain [N]  (N must be > 0)\n");
            exit(1);
        }
    }

    int depth;
    for (depth = 0; depth < n; depth++) {
        printf("Depth %d: PID %d, Parent PID %d\n",
               depth, getpid(), getppid());

        if (depth == n - 1) {
            // Last process in the chain — the leaf
            break;
        }

        int pid = fork();
        if (pid < 0) {
            printf("fork failed at depth %d\n", depth);
            exit(1);
        }

        if (pid > 0) {
            // Parent: wait for child and exit
            wait(0);
            // After child finishes, parent is done too
            // Only the original process (depth 0) prints the total
            if (depth == 0) {
                printf("Total processes in chain: %d\n", n);
            }
            exit(0);
        }

        // Child continues the loop at depth + 1
    }

    // Leaf process
    printf("I am the leaf! No more children.\n");
    exit(0);
}
