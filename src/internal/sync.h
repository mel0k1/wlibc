#pragma once

#include <syscall_arch.h>

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1
#define FUTEX_PRIVATE 128

#define a_cas(p, o, n) __sync_val_compare_and_swap(p, o, n)
#define a_swap(p, v) __atomic_exchange_n(p, v, __ATOMIC_SEQ_CST)
#define a_add(p, v) __atomic_add_fetch(p, v, __ATOMIC_SEQ_CST)
#define a_sub(p, v) __atomic_sub_fetch(p, v, __ATOMIC_SEQ_CST)
#define a_load(p) __atomic_load_n(p, __ATOMIC_SEQ_CST)
#define a_store(p, v) __atomic_store_n(p, v, __ATOMIC_SEQ_CST)

// raw futexes, возвращают errno-код ядра (или 0)
static inline int __futex_wait(volatile int *f, int v)
{
    long r = __syscall4(SYS_futex, (long)f, FUTEX_WAIT | FUTEX_PRIVATE, v, 0);
    return r < 0 ? (int)-r : 0;
}

// ядро будит clear_child_tid без PRIVATE, join обязан ждать так же
static inline int __futex_wait_shared(volatile int *f, int v)
{
    long r = __syscall4(SYS_futex, (long)f, FUTEX_WAIT, v, 0);
    return r < 0 ? (int)-r : 0;
}

static inline int __futex_wake(volatile int *f, int n)
{
    long r = __syscall3(SYS_futex, (long)f, FUTEX_WAKE | FUTEX_PRIVATE, n);
    return r < 0 ? (int)-r : 0;
}
