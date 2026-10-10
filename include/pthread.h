#pragma once

#include <stddef.h>
#include <stdarg.h>
#include <signal.h>

typedef struct pthread *pthread_t;

typedef struct {
    size_t __stack_size;
    int __detach;
} pthread_attr_t;

typedef struct {
    int __lock; // 0 свободен, 1 взят, 2 взят + есть ждущие
} pthread_mutex_t;

typedef struct {
    int __unused;
} pthread_mutexattr_t;

typedef struct {
    int __seq;
    int __waiters;
} pthread_cond_t;

typedef struct {
    int __unused;
} pthread_condattr_t;

typedef int pthread_once_t;

#define PTHREAD_MUTEX_INITIALIZER {0}
#define PTHREAD_COND_INITIALIZER {0, 0}
#define PTHREAD_ONCE_INIT 0

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

#define PTHREAD_STACK_MIN 16384
#define PTHREAD_DESTRUCTOR_ITERATIONS 4

int pthread_create(pthread_t *t, const pthread_attr_t *attr, void *(*fn)(void *),
                   void *arg);
pthread_t pthread_self(void);
int pthread_equal(pthread_t a, pthread_t b);
__attribute__((noreturn)) void pthread_exit(void *res);
int pthread_join(pthread_t t, void **res);
int pthread_detach(pthread_t t);

int pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *a);
int pthread_mutex_destroy(pthread_mutex_t *m);
int pthread_mutex_lock(pthread_mutex_t *m);
int pthread_mutex_trylock(pthread_mutex_t *m);
int pthread_mutex_unlock(pthread_mutex_t *m);

int pthread_cond_init(pthread_cond_t *c, const pthread_condattr_t *a);
int pthread_cond_destroy(pthread_cond_t *c);
int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m);
int pthread_cond_signal(pthread_cond_t *c);
int pthread_cond_broadcast(pthread_cond_t *c);

int pthread_once(pthread_once_t *o, void (*fn)(void));

int pthread_atfork(void (*prepare)(void), void (*parent)(void),
                   void (*child)(void));

int pthread_sigmask(int how, const sigset_t *set, sigset_t *old);
int pthread_kill(pthread_t t, int sig);

int pthread_attr_init(pthread_attr_t *a);
int pthread_attr_destroy(pthread_attr_t *a);
int pthread_attr_setstacksize(pthread_attr_t *a, size_t n);
int pthread_attr_getstacksize(const pthread_attr_t *a, size_t *n);
int pthread_attr_setdetachstate(pthread_attr_t *a, int s);
int pthread_attr_getdetachstate(const pthread_attr_t *a, int *s);
