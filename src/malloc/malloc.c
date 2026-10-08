#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <syscall.h>

// куча: заранее резервируем виртуальное пространство (PROT_NONE) и по мере
// необходимости подключаем страницы через mprotect, поэтому блоки всегда
// лежат непрерывно и соседние свободные можно склеивать

#define ALIGN 16
#define HDR ((size_t)sizeof(blk_t))
#define FTR ((size_t)sizeof(size_t))
#define OVERHEAD (HDR + FTR + 8)
#define MINBLK 32
#define CHUNK ((size_t)65536)
#define RESERVE ((size_t)256 << 20)

typedef struct blk {
    size_t size; // полный размер блока, младший бит = «занят»
    size_t pad;  // держит данные выровненными на 16
} blk_t;

static char *h_base;
static char *h_end;
static char *h_limit;

static size_t blk_need(size_t n)
{
    return ((n + ALIGN - 1) & ~(size_t)(ALIGN - 1)) + OVERHEAD;
}

static blk_t *heap_new_block(size_t need)
{
    if (!h_base) {
        void *m = mmap(0, RESERVE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (m == MAP_FAILED)
            return 0;
        h_base = m;
        h_end = m;
        h_limit = h_base + RESERVE;
    }
    size_t grow = (need + 4095) & ~(size_t)4095;
    if ((size_t)(h_limit - h_end) < grow)
        return 0;
    if (mprotect(h_end, grow, PROT_READ | PROT_WRITE) < 0)
        return 0;
    blk_t *b = (blk_t *)h_end;
    h_end += grow;
    b->size = grow;
    *(size_t *)((char *)b + grow - FTR) = grow;
    return b;
}

static blk_t *find_fit(size_t need)
{
    char *p = h_base;
    while (p && p < h_end) {
        blk_t *b = (blk_t *)p;
        size_t sz = b->size & ~1UL;
        if (!(b->size & 1) && sz >= need)
            return b;
        p += sz;
    }
    return 0;
}

static void blk_use(blk_t *b, size_t need)
{
    size_t total = b->size;
    if (total - need >= MINBLK) {
        blk_t *rest = (blk_t *)((char *)b + need);
        rest->size = total - need;
        *(size_t *)((char *)rest + rest->size - FTR) = rest->size;
        total = need;
    }
    b->size = total | 1;
    *(size_t *)((char *)b + total - FTR) = b->size;
}

void *malloc(size_t n)
{
    if (!n)
        n = 1;
    if (n > ((size_t)-1) / 4) {
        errno = ENOMEM;
        return 0;
    }
    size_t need = blk_need(n);
    blk_t *b = find_fit(need);
    if (!b) {
        size_t grow = need < CHUNK ? CHUNK : need;
        b = heap_new_block(grow);
        if (!b) {
            errno = ENOMEM;
            return 0;
        }
    }
    blk_use(b, need);
    return (char *)b + HDR;
}

void free(void *ptr)
{
    if (!ptr)
        return;
    blk_t *b = (blk_t *)((char *)ptr - HDR);
    b->size &= ~1UL;
    char *nb = (char *)b + b->size;
    if (nb < h_end) {
        blk_t *next = (blk_t *)nb;
        if (!(next->size & 1))
            b->size += next->size;
    }
    if ((char *)b > h_base) {
        size_t psz = *(size_t *)((char *)b - FTR);
        if (!(psz & 1)) {
            blk_t *prev = (blk_t *)((char *)b - psz);
            prev->size += b->size;
            b = prev;
        }
    }
    *(size_t *)((char *)b + b->size - FTR) = b->size;
}

void *calloc(size_t nmemb, size_t size)
{
    if (size && nmemb > (size_t)-1 / size) {
        errno = ENOMEM;
        return 0;
    }
    size_t n = nmemb * size;
    void *p = malloc(n);
    if (p)
        memset(p, 0, n);
    return p;
}

void *realloc(void *ptr, size_t n)
{
    if (!ptr)
        return malloc(n);
    if (!n) {
        free(ptr);
        return 0;
    }
    blk_t *b = (blk_t *)((char *)ptr - HDR);
    size_t cur = (b->size & ~1UL) - OVERHEAD;
    if (n <= cur)
        return ptr;
    size_t need = blk_need(n);
    char *nb = (char *)b + (b->size & ~1UL);
    if (nb < h_end && !(((blk_t *)nb)->size & 1)) {
        size_t merged = (b->size & ~1UL) + ((blk_t *)nb)->size;
        if (merged >= need) {
            b->size = merged;
            *(size_t *)((char *)b + merged - FTR) = b->size;
            blk_use(b, need);
            return ptr;
        }
    }
    void *np = malloc(n);
    if (!np)
        return 0;
    memcpy(np, ptr, cur < n ? cur : n);
    free(ptr);
    return np;
}
