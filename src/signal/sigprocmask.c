#include <signal.h>
#include <syscall.h>

int sigprocmask(int how, const sigset_t *set, sigset_t *old)
{
    return (int)__syscall_ret(__syscall4(SYS_rt_sigprocmask, how, (long)set,
                                         (long)old, 8));
}
