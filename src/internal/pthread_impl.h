#pragma once

#include <stddef.h>
#include <sync.h>

// TCB: лежит по fs:0, сразу после образа TLS-сегмента
struct pthread {
    struct pthread *self; // обязан быть первым (fs:0)
    int tid;              // также futex-слово для join (сбрасывается ядром при выходе)
    int detach;           // 0 = joinable, 1 = detached, 2 = мёртв (ждёт сборки)
    void *(*fn)(void *);
    void *arg;
    void *ret;
    void *map_base; // mmap со стеком
    size_t map_size;
    void *tls_base; // блок TLS (для munmap у join/detach)
    size_t tls_size;
};

#define CLONE_VM 0x100
#define CLONE_FS 0x200
#define CLONE_FILES 0x400
#define CLONE_SIGHAND 0x800
#define CLONE_THREAD 0x10000
#define CLONE_SYSVSEM 0x40000
#define CLONE_SETTLS 0x80000
#define CLONE_PARENT_SETTID 0x100000
#define CLONE_CHILD_CLEARTID 0x200000
#define CLONE_CHILD_SETTID 0x1000000

#define CLONE_THREAD_FLAGS                                                     \
    (CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD |        \
     CLONE_SYSVSEM | CLONE_SETTLS | CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID)

#define ARCH_SET_FS 0x1002

static inline struct pthread *__pthread_self(void)
{
    struct pthread *td;
    __asm__("mov %%fs:0, %0" : "=r"(td));
    return td;
}

// tls.c
extern size_t __tls_data_size;
extern size_t __tls_alloc_size; // P + data + TCB, кратно странице не требуется
void __init_tls(unsigned long *phdr, int phnum, int phent);
struct pthread *__tls_new(void);

// pthread_create.c
__attribute__((noreturn)) void __thread_start(struct pthread *td);

// clone.s / unmapself.s
long __clone(unsigned long flags, void *sp, int *ptid, int *ctid, void *tls);
__attribute__((noreturn)) void __unmapself(void *base, size_t size);
