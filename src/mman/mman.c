#include <errno.h>
#include <sys/mman.h>
#include <syscall.h>

void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
    if (!len) {
        errno = EINVAL;
        return MAP_FAILED;
    }
    return (void *)__syscall_ret(__syscall6(SYS_mmap, (long)addr, (long)len, prot,
                                            flags, fd, off));
}

int munmap(void *addr, size_t len)
{
    return (int)__syscall_ret(__syscall2(SYS_munmap, (long)addr, (long)len));
}

int mprotect(void *addr, size_t len, int prot)
{
    return (int)__syscall_ret(__syscall3(SYS_mprotect, (long)addr, (long)len, prot));
}
