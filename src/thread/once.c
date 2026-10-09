#include <errno.h>
#include <pthread.h>
#include <pthread_impl.h>

// 0 = не запускался, 1 = выполняется, 2 = готово
int pthread_once(pthread_once_t *o, void (*fn)(void))
{
    for (;;) {
        int state = a_load(o);
        if (state == 2)
            return 0;
        if (state == 0 && a_cas(o, 0, 1) == 0) {
            fn();
            a_store(o, 2);
            __futex_wake(o, 0x7fffffff);
            return 0;
        }
        __futex_wait(o, 1);
    }
}

int pthread_attr_init(pthread_attr_t *a)
{
    a->__stack_size = 0;
    a->__detach = 0;
    return 0;
}

int pthread_attr_destroy(pthread_attr_t *a)
{
    (void)a;
    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *a, size_t n)
{
    if (n < PTHREAD_STACK_MIN)
        return EINVAL;
    a->__stack_size = n;
    return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *a, size_t *n)
{
    *n = a->__stack_size;
    return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *a, int s)
{
    if (s != PTHREAD_CREATE_JOINABLE && s != PTHREAD_CREATE_DETACHED)
        return EINVAL;
    a->__detach = s;
    return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *a, int *s)
{
    *s = a->__detach;
    return 0;
}
