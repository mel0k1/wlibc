#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <syscall.h>

ssize_t read(int fd, void *buf, size_t n)
{
    return __syscall_ret(__syscall3(SYS_read, fd, (long)buf, (long)n));
}

ssize_t write(int fd, const void *buf, size_t n)
{
    return __syscall_ret(__syscall3(SYS_write, fd, (long)buf, (long)n));
}

int close(int fd)
{
    return (int)__syscall_ret(__syscall1(SYS_close, fd));
}

off_t lseek(int fd, off_t off, int whence)
{
    return __syscall_ret(__syscall3(SYS_lseek, fd, off, whence));
}
