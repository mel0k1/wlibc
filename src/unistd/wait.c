#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <syscall.h>

pid_t waitpid(pid_t pid, int *status, int options)
{
    return (pid_t)__syscall_ret(__syscall4(SYS_wait4, pid, (long)status,
                                           options, 0));
}

pid_t wait(int *status)
{
    return waitpid(-1, status, 0);
}
