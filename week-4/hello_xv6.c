// hello_xv6.c — Week 4, Problem 1
//
// A simple xv6 user program that:
//   1. Prints a greeting with xv6's name and the author's name
//   2. Prints the number of command-line arguments
//   3. Prints each argument on its own line
//   4. Prints the number of system calls defined in kernel/syscall.h
//
// To add to xv6:
//   1. Place this file in user/hello_xv6.c
//   2. Add $U/_hello_xv6\ to UPROGS in the Makefile
//   3. make clean && make && make qemu
//   4. At the $ prompt: hello_xv6

#include "kernel/types.h"
#include "user/user.h"

// Counted manually from kernel/syscall.h:
// SYS_fork=1, SYS_exit=2, SYS_wait=3, SYS_pipe=4, SYS_read=5,
// SYS_kill=6, SYS_exec=7, SYS_fstat=8, SYS_chdir=9, SYS_dup=10,
// SYS_getpid=11, SYS_sbrk=12, SYS_sleep=13, SYS_uptime=14,
// SYS_open=15, SYS_write=16, SYS_mknod=17, SYS_unlink=18,
// SYS_link=19, SYS_mkdir=20, SYS_close=21
// Total: 21 system calls
#define NUM_SYSCALLS 21

int main(int argc, char *argv[]) {
    printf("Hello, xv6! This is Himanshu.\n");

    // argc includes the program name, so actual arguments = argc - 1
    printf("Number of arguments: %d\n", argc - 1);

    for (int i = 1; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    printf("xv6 knows %d system calls.\n", NUM_SYSCALLS);

    exit(0);
}
