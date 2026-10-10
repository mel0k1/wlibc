#pragma once

#include <sys/types.h>

typedef long clock_t;
typedef int clockid_t;

#define CLOCKS_PER_SEC 1000000L

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3
#define CLOCK_MONOTONIC_RAW 4
#define CLOCK_REALTIME_COARSE 5
#define CLOCK_MONOTONIC_COARSE 6
#define CLOCK_BOOTTIME 7
#define CLOCK_REALTIME_ALARM 8
#define CLOCK_BOOTTIME_ALARM 9
#define CLOCK_SGI_CYCLE 10
#define CLOCK_TAI 11

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    long tm_gmtoff;
    const char *tm_zone;
};

clock_t clock(void);
time_t time(time_t *t);
int clock_gettime(clockid_t clk, struct timespec *ts);
int clock_getres(clockid_t clk, struct timespec *res);
int nanosleep(const struct timespec *req, struct timespec *rem);

double difftime(time_t a, time_t b);
time_t mktime(struct tm *tm);
time_t timegm(struct tm *tm);
char *asctime(const struct tm *tm);
char *asctime_r(const struct tm *tm, char *buf);
char *ctime(const time_t *t);
char *ctime_r(const time_t *t, char *buf);
struct tm *gmtime(const time_t *t);
struct tm *gmtime_r(const time_t *t, struct tm *tm);
struct tm *localtime(const time_t *t);
struct tm *localtime_r(const time_t *t, struct tm *tm);
size_t strftime(char *s, size_t max, const char *fmt, const struct tm *tm);
