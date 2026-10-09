#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <syscall.h>
#include <pthread_impl.h>

// detach: 0 = joinable, 1 = detached, 2 = мёртв; слово tid очищает ядро при выходе
__attribute__((noreturn)) void pthread_exit(void *res)
{
    struct pthread *self = __pthread_self();
    self->ret = res;
    int prev = a_swap(&self->detach, 2);
    if (prev == 1)
        __unmapself(self->map_base, self->map_size);
    __syscall1(SYS_exit, 0);
    for (;;)
        ;
}

int pthread_join(pthread_t t, void **res)
{
    if (!t)
        return EINVAL;
    if (t == pthread_self())
        return EDEADLK;
    int tid = a_load(&t->tid);
    while (tid) {
        __futex_wait_shared(&t->tid, tid);
        tid = a_load(&t->tid);
    }
    if (a_cas(&t->detach, 2, 3) != 2)
        return EINVAL;
    if (res)
        *res = t->ret;
    munmap(t->map_base, t->map_size);
    munmap(t->tls_base, __tls_alloc_size);
    return 0;
}

int pthread_detach(pthread_t t)
{
    if (!t)
        return EINVAL;
    int prev = a_swap(&t->detach, 1);
    if (prev == 1)
        return EINVAL;
    if (prev == 2) {
        munmap(t->map_base, t->map_size);
        munmap(t->tls_base, __tls_alloc_size);
    }
    return 0;
}

pthread_t pthread_self(void)
{
    return __pthread_self();
}

int pthread_equal(pthread_t a, pthread_t b)
{
    return a == b;
}
