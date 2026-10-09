#include <sys/time.h>
#include <vdso.h>
#include <syscall.h>

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    if (__vdso_gettimeofday_f && __vdso_gettimeofday_f(tv, tz) == 0)
        return 0;
    return (int)__syscall_ret(__syscall2(SYS_gettimeofday, (long)tv,
                                         (long)tz));
}
