#include <errno.h>
#include <pthread.h>
#include <pthread_impl.h>

int pthread_cond_init(pthread_cond_t *c, const pthread_condattr_t *a)
{
    (void)a;
    c->__seq = 0;
    c->__waiters = 0;
    return 0;
}

int pthread_cond_destroy(pthread_cond_t *c)
{
    (void)c;
    return 0;
}

// seq-протокол: лишние пробуждения допустимы, корректность даёт
// перепроверка предиката в цикле while (!cond) pthread_cond_wait(...)
int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m)
{
    int seq = c->__seq;
    a_add(&c->__waiters, 1);
    pthread_mutex_unlock(m);
    while (seq == a_load(&c->__seq))
        __futex_wait(&c->__seq, seq);
    a_sub(&c->__waiters, 1);
    pthread_mutex_lock(m);
    return 0;
}

int pthread_cond_signal(pthread_cond_t *c)
{
    if (a_load(&c->__waiters)) {
        a_add(&c->__seq, 1);
        __futex_wake(&c->__seq, 1);
    }
    return 0;
}

int pthread_cond_broadcast(pthread_cond_t *c)
{
    if (a_load(&c->__waiters)) {
        a_add(&c->__seq, 1);
        __futex_wake(&c->__seq, 0x7fffffff);
    }
    return 0;
}
