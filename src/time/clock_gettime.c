#include <time.h>
#include <vdso.h>
#include <syscall.h>

int clock_gettime(clockid_t clk, struct timespec *ts)
{
    if (__vdso_clock_gettime_f && __vdso_clock_gettime_f(clk, ts) == 0)
        return 0;
    return (int)__syscall_ret(__syscall2(SYS_clock_gettime, clk, (long)ts));
}

int clock_getres(clockid_t clk, struct timespec *res)
{
    if (__vdso_clock_getres_f && __vdso_clock_getres_f(clk, res) == 0)
        return 0;
    return (int)__syscall_ret(__syscall2(SYS_clock_getres, clk, (long)res));
}
