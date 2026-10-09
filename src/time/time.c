#include <time.h>
#include <vdso.h>
#include <syscall.h>

time_t time(time_t *t)
{
    if (__vdso_time_f)
        return (time_t)__vdso_time_f(t);
    long r = __syscall_ret(__syscall1(SYS_time, (long)t));
    return (time_t)r;
}
