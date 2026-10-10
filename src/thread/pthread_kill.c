#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <syscall.h>
#include <pthread_impl.h>

int pthread_kill(pthread_t t, int sig)
{
    if ((unsigned)sig >= _NSIG)
        return EINVAL;
    pid_t pid = (pid_t)__syscall0(SYS_getpid);
    return (int)__syscall_ret(__syscall3(SYS_tgkill, pid, t->tid, sig));
}
