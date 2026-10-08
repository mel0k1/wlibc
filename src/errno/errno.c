#include <errno.h>
#include <stdio.h>
#include <string.h>

static int errno_val;

int *__errno_location(void)
{
    return &errno_val;
}

static const char *const msgs[] = {
    [EPERM] = "Operation not permitted",
    [ENOENT] = "No such file or directory",
    [ESRCH] = "No such process",
    [EINTR] = "Interrupted system call",
    [EIO] = "I/O error",
    [ENXIO] = "No such device or address",
    [E2BIG] = "Argument list too long",
    [ENOEXEC] = "Exec format error",
    [EBADF] = "Bad file descriptor",
    [ECHILD] = "No child process",
    [EAGAIN] = "Resource temporarily unavailable",
    [ENOMEM] = "Out of memory",
    [EACCES] = "Permission denied",
    [EFAULT] = "Bad address",
    [ENOTBLK] = "Block device required",
    [EBUSY] = "Resource busy",
    [EEXIST] = "File exists",
    [EXDEV] = "Cross-device link",
    [ENODEV] = "No such device",
    [ENOTDIR] = "Not a directory",
    [EISDIR] = "Is a directory",
    [EINVAL] = "Invalid argument",
    [ENFILE] = "Too many open files in system",
    [EMFILE] = "Too many open files",
    [ENOTTY] = "Not a tty",
    [ETXTBSY] = "Text file busy",
    [EFBIG] = "File too large",
    [ENOSPC] = "No space left on device",
    [ESPIPE] = "Illegal seek",
    [EROFS] = "Read-only file system",
    [EMLINK] = "Too many links",
    [EPIPE] = "Broken pipe",
    [EDOM] = "Domain error",
    [ERANGE] = "Result not representable",
    [EDEADLK] = "Resource deadlock would occur",
    [ENAMETOOLONG] = "Filename too long",
    [ENOLCK] = "No locks available",
    [ENOSYS] = "Function not implemented",
    [ENOTEMPTY] = "Directory not empty",
    [ELOOP] = "Symbolic link loop",
    [ENOMSG] = "No message of desired type",
    [EIDRM] = "Identifier removed",
    [EOVERFLOW] = "Value too large for data type",
    [EBADFD] = "File descriptor in bad state",
    [EADDRINUSE] = "Address in use",
    [EADDRNOTAVAIL] = "Address not available",
    [ENETDOWN] = "Network is down",
    [ENETUNREACH] = "Network unreachable",
    [ECONNRESET] = "Connection reset by peer",
    [ENOBUFS] = "No buffer space available",
    [EISCONN] = "Socket is connected",
    [ENOTCONN] = "Socket not connected",
    [ETIMEDOUT] = "Operation timed out",
    [ECONNREFUSED] = "Connection refused",
    [EHOSTUNREACH] = "Host is unreachable",
    [EALREADY] = "Operation already in progress",
    [EINPROGRESS] = "Operation in progress",
};

const char *strerror(int e)
{
    if (e >= 0 && (size_t)e < sizeof(msgs) / sizeof(*msgs) && msgs[e])
        return msgs[e];
    static char buf[32];
    snprintf(buf, sizeof(buf), "Unknown error %d", e);
    return buf;
}
