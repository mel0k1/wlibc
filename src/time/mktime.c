#include <time.h>
#include <time_impl.h>

long long __tm_to_secs(const struct tm *tm)
{
    long long y = tm->tm_year + 1900LL;
    long long m = tm->tm_mon;
    y += m / 12;
    m %= 12;
    if (m < 0) {
        m += 12;
        y--;
    }
    long long days = __days_from_civil(y, (int)m + 1, 1);
    return days * 86400 + (tm->tm_mday - 1) * 86400 + tm->tm_hour * 3600 +
           tm->tm_min * 60 + tm->tm_sec;
}

time_t timegm(struct tm *tm)
{
    long long t = __tm_to_secs(tm);
    __secs_to_tm(t, tm);
    return (time_t)t;
}

// локальная зона всегда UTC
time_t mktime(struct tm *tm)
{
    return timegm(tm);
}
