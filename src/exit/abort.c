#include <stdlib.h>
#include <syscall.h>

void abort(void)
{
    __syscall2(SYS_kill, __syscall0(SYS_getpid), 6); // SIGABRT
    __syscall1(SYS_exit_group, 127);
    for (;;)
        __syscall1(SYS_exit, 127);
}
