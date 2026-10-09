#include <errno.h>
#include <pthread.h>
#include <libc.h>

#define ATFORK_MAX 32

static void (*atfork_prepare[ATFORK_MAX])(void);
static void (*atfork_parent[ATFORK_MAX])(void);
static void (*atfork_child[ATFORK_MAX])(void);
static int atfork_n;

int pthread_atfork(void (*prepare)(void), void (*parent)(void),
                   void (*child)(void))
{
    if (atfork_n == ATFORK_MAX) {
        errno = ENOMEM;
        return -1;
    }
    atfork_prepare[atfork_n] = prepare;
    atfork_parent[atfork_n] = parent;
    atfork_child[atfork_n] = child;
    atfork_n++;
    return 0;
}

void __atfork_run(int which)
{
    void (**fns)(void) = which == 0 ? atfork_prepare
                       : which == 1 ? atfork_parent
                                    : atfork_child;
    if (which == 0) {
        for (int i = atfork_n - 1; i >= 0; i--)
            if (fns[i])
                fns[i]();
    } else {
        for (int i = 0; i < atfork_n; i++)
            if (fns[i])
                fns[i]();
    }
}
