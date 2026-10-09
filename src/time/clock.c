#include <sys/times.h>
#include <time.h>
#include <syscall.h>

clock_t clock(void)
{
    struct tms t;
    if (__syscall_ret(__syscall1(SYS_times, (long)&t)) == -1)
        return (clock_t)-1;
    // SYS_times считает в тиках по 1/100 с
    return (clock_t)((t.tms_utime + t.tms_stime) * (CLOCKS_PER_SEC / 100));
}
