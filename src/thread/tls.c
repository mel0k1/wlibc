#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <syscall.h>
#include <pthread_impl.h>

// x86_64 TLS Variant II: линкер считает tpoff = roundup(memsz, align) - var,
// поэтому TCB ставится на data_start + roundup(memsz, align);
// фронт-паддинг P выравнивает TCB на 16, сохраняя выравнивание данных

#define PT_TLS 7

size_t __tls_data_size;
size_t __tls_alloc_size;
static size_t tls_dist;      // data -> TCB: roundup(memsz, align)
static size_t tls_image_off; // паддинг P до данных
static const void *tls_image;
static size_t tls_image_len;

static size_t align_up(size_t n, size_t a)
{
    return (n + a - 1) & ~(a - 1);
}

void __init_tls(unsigned long *phdr, int phnum, int phent)
{
    size_t memsz = 0, filesz = 0, align = 1;
    const void *img = 0;

    if (phent == 56) {
        for (int i = 0; i < phnum; i++) {
            unsigned long *p = phdr + (size_t)i * 7;
            if ((int)p[0] == PT_TLS) {
                img = (const void *)p[2]; // p_vaddr, бинарник не-PIE
                filesz = p[4];
                memsz = p[5];
                align = p[6];
                break;
            }
        }
    }
    if (align < 1)
        align = 1;

    size_t dist = (memsz + align - 1) & ~(align - 1);
    size_t pad = (16 - (dist % 16)) % 16;
    tls_dist = dist;

    __tls_data_size = memsz;
    tls_image_off = pad;
    tls_image = img;
    tls_image_len = filesz;
    __tls_alloc_size = align_up(pad + dist + sizeof(struct pthread), 4096);

    struct pthread *td = __tls_new();
    td->tid = (int)__syscall1(SYS_set_tid_address, (long)&td->tid);
    __syscall2(SYS_arch_prctl, ARCH_SET_FS, (long)td);
}

struct pthread *__tls_new(void)
{
    unsigned char *mem = mmap(0, __tls_alloc_size, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED)
        return 0;
    if (tls_image_len)
        memcpy(mem + tls_image_off, tls_image, tls_image_len);
    struct pthread *td =
        (struct pthread *)(mem + tls_image_off + tls_dist);
    td->self = td;
    td->tls_base = mem;
    return td;
}
