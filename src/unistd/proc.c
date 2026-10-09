#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <syscall.h>

pid_t getpid(void)
{
    return (pid_t)__syscall_ret(__syscall0(SYS_getpid));
}

pid_t getppid(void)
{
    return (pid_t)__syscall_ret(__syscall0(SYS_getppid));
}

uid_t getuid(void)
{
    return (uid_t)__syscall_ret(__syscall0(SYS_getuid));
}

gid_t getgid(void)
{
    return (gid_t)__syscall_ret(__syscall0(SYS_getgid));
}

int unlink(const char *path)
{
    return (int)__syscall_ret(__syscall1(SYS_unlink, (long)path));
}

int pipe(int fds[2])
{
    return (int)__syscall_ret(__syscall1(SYS_pipe, (long)fds));
}

unsigned sleep(unsigned seconds)
{
    struct timespec req = { (long)seconds, 0 }, rem;
    rem.tv_sec = 0;
    rem.tv_nsec = 0;
    while (nanosleep(&req, &rem) == -1 && errno == EINTR)
        req = rem;
    return (unsigned)rem.tv_sec;
}
