#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <syscall.h>

pid_t getpid(void)
{
    return (pid_t)__syscall_ret(__syscall0(SYS_getpid));
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

unsigned sleep(unsigned seconds)
{
    struct ts {
        long sec;
        long nsec;
    } req = { (long)seconds, 0 }, rem;
    for (;;) {
        rem.sec = 0;
        rem.nsec = 0;
        long r = __syscall2(SYS_nanosleep, (long)&req, (long)&rem);
        if (r != -1 || errno != EINTR)
            return (unsigned)rem.sec;
        req = rem;
    }
}
