#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <syscall.h>

int kill(pid_t pid, int sig)
{
    return (int)__syscall_ret(__syscall2(SYS_kill, pid, sig));
}

int raise(int sig)
{
    pid_t pid = (pid_t)__syscall0(SYS_getpid);
    pid_t tid = (pid_t)__syscall0(SYS_gettid);
    return (int)__syscall_ret(__syscall3(SYS_tgkill, pid, tid, sig));
}
