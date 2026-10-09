#include <signal.h>
#include <syscall.h>

int sigsuspend(const sigset_t *mask)
{
    return (int)__syscall_ret(__syscall2(SYS_rt_sigsuspend, (long)mask, 8));
}
