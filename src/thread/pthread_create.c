#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <syscall.h>
#include <pthread_impl.h>

#define DEFAULT_STACK ((size_t)262144)
#define GUARD_SIZE 4096

__attribute__((noreturn)) void __thread_start(struct pthread *td)
{
    td->ret = td->fn(td->arg);
    pthread_exit(td->ret);
}

int pthread_create(pthread_t *res, const pthread_attr_t *attr, void *(*fn)(void *),
                   void *arg)
{
    size_t stack_size = DEFAULT_STACK;
    int detach = 0;
    if (attr) {
        if (attr->__stack_size)
            stack_size = attr->__stack_size;
        detach = attr->__detach;
    }

    size_t map_size = stack_size + GUARD_SIZE;
    map_size = (map_size + 4095) & ~(size_t)4095;
    unsigned char *stack = mmap(0, map_size, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (stack == MAP_FAILED)
        return EAGAIN;
    if (mprotect(stack, GUARD_SIZE, PROT_NONE) < 0) {
        munmap(stack, map_size);
        return EAGAIN;
    }

    struct pthread *td = __tls_new();
    if (!td) {
        munmap(stack, map_size);
        return EAGAIN;
    }
    td->fn = fn;
    td->arg = arg;
    td->detach = detach;
    td->map_base = stack;
    td->map_size = map_size;

    unsigned char *sp = stack + map_size;
    sp -= 16;
    ((void **)sp)[0] = (void *)__thread_start;
    ((void **)sp)[1] = td;

    long r = __clone(CLONE_THREAD_FLAGS, sp, 0, &td->tid, td);
    if (r < 0) {
        munmap(td->tls_base, __tls_alloc_size);
        munmap(stack, map_size);
        return EAGAIN;
    }
    // clone вернул tid ребёнка родителю: CHILD_SETTID пишет с задержкой,
    // а join обязан видеть tid != 0 сразу
    td->tid = (int)r;
    *res = td;
    return 0;
}
