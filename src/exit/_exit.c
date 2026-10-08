#include <stdlib.h>
#include <syscall.h>

void _exit(int code)
{
    __syscall1(SYS_exit_group, code);
    for (;;)
        __syscall1(SYS_exit, code);
}
