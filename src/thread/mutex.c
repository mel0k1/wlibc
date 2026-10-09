#include <errno.h>
#include <pthread.h>
#include <pthread_impl.h>

int pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *a)
{
    (void)a;
    m->__lock = 0;
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *m)
{
    (void)m;
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *m)
{
    if (a_cas(&m->__lock, 0, 1) == 0)
        return 0;
    // 2 = «взят и есть ждущие», чтобы unlock делал futex_wake только при нужде
    while (a_swap(&m->__lock, 2))
        __futex_wait(&m->__lock, 2);
    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *m)
{
    return a_cas(&m->__lock, 0, 1) == 0 ? 0 : EBUSY;
}

int pthread_mutex_unlock(pthread_mutex_t *m)
{
    if (a_swap(&m->__lock, 0) == 2)
        __futex_wake(&m->__lock, 1);
    return 0;
}
