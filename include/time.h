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

clock_t clock(void);
time_t time(time_t *t);
int clock_gettime(clockid_t clk, struct timespec *ts);
int clock_getres(clockid_t clk, struct timespec *res);
int nanosleep(const struct timespec *req, struct timespec *rem);
