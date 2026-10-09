#include <errno.h>
#include <libc.h>
#include <pthread_impl.h>
#include <syscall.h>
#include <unistd.h>

#define SIGCHLD 17

void __atfork_run(int which);

pid_t fork(void)
{
    __atfork_run(0);

    long ret = __syscall5(SYS_clone, SIGCHLD, 0, 0, 0, 0);

    if (ret == 0) {
        struct pthread *td = __pthread_self();
        td->tid = (int)__syscall1(SYS_set_tid_address, (long)&td->tid);
        __malloc_fork_child();
        __stdio_fork_child();
        __atfork_run(2);
    } else if (ret > 0) {
        __atfork_run(1);
    }

    return (pid_t)__syscall_ret(ret);
}
