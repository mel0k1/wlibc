#pragma once

#include <errno.h>

#define SYS_read 0
#define SYS_write 1
#define SYS_open 2
#define SYS_close 3
#define SYS_lseek 8
#define SYS_mmap 9
#define SYS_mprotect 10
#define SYS_munmap 11
#define SYS_ioctl 16
#define SYS_pipe 22
#define SYS_nanosleep 35
#define SYS_getpid 39
#define SYS_kill 62
#define SYS_uname 63
#define SYS_unlink 87
#define SYS_getuid 102
#define SYS_getgid 104
#define SYS_geteuid 107
#define SYS_getegid 108
#define SYS_exit 60
#define SYS_exit_group 231

static inline long __syscall_ret(unsigned long r)
{
    if (r > (unsigned long)-4096) {
        errno = -(long)r;
        return -1;
    }
    return (long)r;
}

static inline long __syscall0(long n)
{
    unsigned long ret;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(n)
                         : "rcx", "r11", "memory");
    return (long)ret;
}

static inline long __syscall1(long n, long a1)
{
    unsigned long ret;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(n), "D"(a1)
                         : "rcx", "r11", "memory");
    return (long)ret;
}

static inline long __syscall2(long n, long a1, long a2)
{
    unsigned long ret;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(n), "D"(a1), "S"(a2)
                         : "rcx", "r11", "memory");
    return (long)ret;
}

static inline long __syscall3(long n, long a1, long a2, long a3)
{
    unsigned long ret;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(n), "D"(a1), "S"(a2), "d"(a3)
                         : "rcx", "r11", "memory");
    return (long)ret;
}

static inline long __syscall4(long n, long a1, long a2, long a3, long a4)
{
    unsigned long ret;
    register long r10 __asm__("r10") = a4;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
                         : "rcx", "r11", "memory");
    return (long)ret;
}

static inline long __syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
    unsigned long ret;
    register long r10 __asm__("r10") = a4;
    register long r8 __asm__("r8") = a5;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
                         : "rcx", "r11", "memory");
    return (long)ret;
}

static inline long __syscall6(long n, long a1, long a2, long a3, long a4, long a5,
                              long a6)
{
    unsigned long ret;
    register long r10 __asm__("r10") = a4;
    register long r8 __asm__("r8") = a5;
    register long r9 __asm__("r9") = a6;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8),
                           "r"(r9)
                         : "rcx", "r11", "memory");
    return (long)ret;
}
