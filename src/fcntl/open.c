#include <fcntl.h>
#include <stdarg.h>
#include <syscall.h>

int open(const char *path, int flags, ...)
{
    unsigned int mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = (unsigned int)va_arg(ap, int);
        va_end(ap);
    }
    return (int)__syscall_ret(__syscall3(SYS_open, (long)path, flags, (long)mode));
}
