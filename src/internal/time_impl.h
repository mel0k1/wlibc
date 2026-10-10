#pragma once

#include <time.h>

// дней с 1970-01-01 до y-m-d, пролептический григорианский календарь
static inline long long __days_from_civil(long long y, int m, int d)
{
    y -= m <= 2;
    long long era = (y >= 0 ? y : y - 399) / 400;
    unsigned yoe = (unsigned)(y - era * 400);
    unsigned doy = (153u * (unsigned)(m + (m > 2 ? -3 : 9)) + 2) / 5 +
                   (unsigned)d - 1;
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (long long)doe - 719468;
}

void __secs_to_tm(long long t, struct tm *tm);
long long __tm_to_secs(const struct tm *tm);
