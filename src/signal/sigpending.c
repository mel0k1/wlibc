#include <signal.h>
#include <syscall.h>

int sigpending(sigset_t *set)
{
    return (int)__syscall_ret(__syscall2(SYS_rt_sigpending, (long)set, 8));
}
