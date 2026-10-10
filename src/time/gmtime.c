#include <time.h>
#include <time_impl.h>

void __secs_to_tm(long long t, struct tm *tm)
{
    long long days = t / 86400;
    long long rem = t % 86400;
    if (rem < 0) {
        rem += 86400;
        days--;
    }
    int wday = (int)((days + 4) % 7); // 1970-01-01 — четверг
    if (wday < 0)
        wday += 7;

    long long z = days + 719468;
    long long era = (z >= 0 ? z : z - 146096) / 146097;
    unsigned doe = (unsigned)(z - era * 146097);
    unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long long y = era * 400 + yoe;
    unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    unsigned mp = (5 * doy + 2) / 153;
    unsigned d = doy - (153 * mp + 2) / 5 + 1;
    unsigned m = mp + (mp < 10 ? 3 : -9);
    y += m <= 2;

    tm->tm_year = (int)(y - 1900);
    tm->tm_mon = (int)m - 1;
    tm->tm_mday = (int)d;
    tm->tm_hour = (int)(rem / 3600);
    tm->tm_min = (int)(rem / 60 % 60);
    tm->tm_sec = (int)(rem % 60);
    tm->tm_wday = wday;
    tm->tm_yday = (int)(days - __days_from_civil(y, 1, 1));
    tm->tm_isdst = 0;
    tm->tm_gmtoff = 0;
    tm->tm_zone = "UTC";
}

struct tm *gmtime_r(const time_t *t, struct tm *tm)
{
    __secs_to_tm(*t, tm);
    return tm;
}

static __thread struct tm gm_buf;

struct tm *gmtime(const time_t *t)
{
    return gmtime_r(t, &gm_buf);
}
