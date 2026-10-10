#include <time.h>

// TZ не разбирается, локальное время совпадает с UTC
struct tm *localtime_r(const time_t *t, struct tm *tm)
{
    return gmtime_r(t, tm);
}

static __thread struct tm lt_buf;

struct tm *localtime(const time_t *t)
{
    return localtime_r(t, &lt_buf);
}
