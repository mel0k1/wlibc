#include <string.h>
#include <time.h>

static const char wday_full[7][10] = { "Sunday", "Monday", "Tuesday",
                                       "Wednesday", "Thursday", "Friday",
                                       "Saturday" };
static const char wday_abbr[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri",
                                      "Sat" };
static const char mon_full[12][10] = { "January", "February", "March",
                                       "April", "May", "June", "July",
                                       "August", "September", "October",
                                       "November", "December" };
static const char mon_abbr[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                      "Jul", "Aug", "Sep", "Oct", "Nov",
                                      "Dec" };

struct out {
    char *p;
    char *end;
    int truncated;
};

static void oput(struct out *o, const char *s, size_t n)
{
    if ((size_t)(o->end - o->p) < n + 1) {
        o->truncated = 1;
        return;
    }
    memcpy(o->p, s, n);
    o->p += n;
}

static void onum(struct out *o, long v, int width, char pad)
{
    char tmp[24];
    int n = 0;
    unsigned long u = v < 0 ? -(unsigned long)v : (unsigned long)v;
    do {
        tmp[n++] = (char)('0' + u % 10);
        u /= 10;
    } while (u);
    if (v < 0)
        tmp[n++] = '-';
    while (width-- > n)
        oput(o, &pad, 1);
    while (n)
        oput(o, &tmp[--n], 1);
}

static int hour12(int h)
{
    int r = h % 12;
    return r ? r : 12;
}

static void fmt_run(struct out *o, const char *fmt, const struct tm *tm)
{
    while (*fmt && !o->truncated) {
        if (*fmt != '%') {
            oput(o, fmt++, 1);
            continue;
        }
        const char *f = fmt + 1;
        if (*f == 'E' || *f == 'O')
            f++;
        switch (*f) {
        case 'a':
            oput(o, wday_abbr[tm->tm_wday % 7], 3);
            break;
        case 'A':
            oput(o, wday_full[tm->tm_wday % 7],
                 strlen(wday_full[tm->tm_wday % 7]));
            break;
        case 'b':
        case 'h':
            oput(o, mon_abbr[tm->tm_mon % 12], 3);
            break;
        case 'B':
            oput(o, mon_full[tm->tm_mon % 12],
                 strlen(mon_full[tm->tm_mon % 12]));
            break;
        case 'c':
            fmt_run(o, "%a %b %e %H:%M:%S %Y", tm);
            break;
        case 'C':
            onum(o, (tm->tm_year + 1900L) / 100, 2, '0');
            break;
        case 'd':
            onum(o, tm->tm_mday, 2, '0');
            break;
        case 'D':
            fmt_run(o, "%m/%d/%y", tm);
            break;
        case 'e':
            onum(o, tm->tm_mday, 2, ' ');
            break;
        case 'F':
            fmt_run(o, "%Y-%m-%d", tm);
            break;
        case 'H':
            onum(o, tm->tm_hour, 2, '0');
            break;
        case 'I':
            onum(o, hour12(tm->tm_hour), 2, '0');
            break;
        case 'j':
            onum(o, tm->tm_yday + 1, 3, '0');
            break;
        case 'm':
            onum(o, tm->tm_mon + 1, 2, '0');
            break;
        case 'M':
            onum(o, tm->tm_min, 2, '0');
            break;
        case 'n':
            oput(o, "\n", 1);
            break;
        case 'p':
            oput(o, tm->tm_hour < 12 ? "AM" : "PM", 2);
            break;
        case 'r':
            fmt_run(o, "%I:%M:%S %p", tm);
            break;
        case 'R':
            fmt_run(o, "%H:%M", tm);
            break;
        case 'S':
            onum(o, tm->tm_sec, 2, '0');
            break;
        case 't':
            oput(o, "\t", 1);
            break;
        case 'T':
            fmt_run(o, "%H:%M:%S", tm);
            break;
        case 'u':
            onum(o, tm->tm_wday ? tm->tm_wday : 7, 1, '0');
            break;
        case 'U':
            onum(o, (tm->tm_yday + 7 - tm->tm_wday) / 7, 2, '0');
            break;
        case 'w':
            onum(o, tm->tm_wday, 1, '0');
            break;
        case 'W':
            onum(o, (tm->tm_yday + 7 - (tm->tm_wday + 6) % 7) / 7, 2, '0');
            break;
        case 'x':
            fmt_run(o, "%m/%d/%y", tm);
            break;
        case 'X':
            fmt_run(o, "%H:%M:%S", tm);
            break;
        case 'y':
            onum(o, (tm->tm_year + 1900L) % 100, 2, '0');
            break;
        case 'Y':
            onum(o, tm->tm_year + 1900L, 4, '0');
            break;
        case 'z': {
            long off = tm->tm_gmtoff;
            if (off < 0)
                off = -off;
            char z[5] = { tm->tm_gmtoff < 0 ? '-' : '+',
                          (char)('0' + off / 36000 % 10),
                          (char)('0' + off / 3600 % 10),
                          (char)('0' + off / 600 % 10),
                          (char)('0' + off / 60 % 10) };
            oput(o, z, 5);
            break;
        }
        case 'Z':
            oput(o, tm->tm_zone ? tm->tm_zone : "UTC",
                 strlen(tm->tm_zone ? tm->tm_zone : "UTC"));
            break;
        case '%':
            oput(o, "%", 1);
            break;
        default:
            oput(o, fmt, (size_t)(f - fmt) + 1);
            break;
        }
        fmt = f + 1;
    }
}

size_t strftime(char *s, size_t max, const char *fmt, const struct tm *tm)
{
    struct out o = { s, s + max, 0 };
    fmt_run(&o, fmt, tm);
    if (o.truncated)
        return 0;
    *o.p = 0;
    return (size_t)(o.p - s);
}
