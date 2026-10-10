#include <time.h>

static const char wdays[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri",
                                  "Sat" };
static const char months[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static char *put2(char *p, int v)
{
    p[0] = (char)('0' + v / 10 % 10);
    p[1] = (char)('0' + v % 10);
    return p + 2;
}

char *asctime_r(const struct tm *tm, char *buf)
{
    const char *wd = wdays[tm->tm_wday % 7];
    const char *mo = months[tm->tm_mon % 12];
    char *p = buf;

    *p++ = wd[0];
    *p++ = wd[1];
    *p++ = wd[2];
    *p++ = ' ';
    *p++ = mo[0];
    *p++ = mo[1];
    *p++ = mo[2];
    *p++ = ' ';
    if (tm->tm_mday < 10) {
        *p++ = ' ';
        *p++ = (char)('0' + tm->tm_mday);
    } else {
        p = put2(p, tm->tm_mday);
    }
    *p++ = ' ';
    p = put2(p, tm->tm_hour);
    *p++ = ':';
    p = put2(p, tm->tm_min);
    *p++ = ':';
    p = put2(p, tm->tm_sec);
    *p++ = ' ';

    long yr = tm->tm_year + 1900L;
    if (yr < 0) {
        *p++ = '-';
        yr = -yr;
    }
    char tmp[12];
    int n = 0;
    do {
        tmp[n++] = (char)('0' + yr % 10);
        yr /= 10;
    } while (yr);
    while (n)
        *p++ = tmp[--n];

    *p++ = '\n';
    *p = 0;
    return buf;
}

static __thread char as_buf[40];
static __thread char ctime_buf[40];

char *asctime(const struct tm *tm)
{
    return asctime_r(tm, as_buf);
}

char *ctime_r(const time_t *t, char *buf)
{
    struct tm tm;
    return asctime_r(gmtime_r(t, &tm), buf);
}

char *ctime(const time_t *t)
{
    return ctime_r(t, ctime_buf);
}
