#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

extern int __vdso_active;

int main(void)
{
    // vDSO обязан найтись на linux/x86_64
    assert(__vdso_active == 1);

    // CLOCK_REALTIME — разумная дата
    struct timespec ts;
    assert(clock_gettime(CLOCK_REALTIME, &ts) == 0);
    assert(ts.tv_sec > 1600000000);
    assert(ts.tv_nsec >= 0 && ts.tv_nsec < 1000000000L);

    // CLOCK_MONOTONIC не течёт назад
    struct timespec m1, m2;
    assert(clock_gettime(CLOCK_MONOTONIC, &m1) == 0);
    assert(clock_gettime(CLOCK_MONOTONIC, &m2) == 0);
    assert(m2.tv_sec > m1.tv_sec ||
           (m2.tv_sec == m1.tv_sec && m2.tv_nsec >= m1.tv_nsec));

    // gettimeofday согласован с time()
    struct timeval tv;
    assert(gettimeofday(&tv, NULL) == 0);
    time_t t = time(NULL);
    assert(t >= tv.tv_sec && (long)t - (long)tv.tv_sec <= 1);
    time_t t2;
    assert(time(&t2) == t2 && t2 >= tv.tv_sec);

    // clock_getres: разрешение 1 нс
    struct timespec res;
    assert(clock_getres(CLOCK_REALTIME, &res) == 0);
    assert(res.tv_sec == 0 && res.tv_nsec == 1);

    // clock()
    clock_t c = clock();
    assert(c >= 0);

    // nanosleep: ~50мс по MONOTONIC
    assert(clock_gettime(CLOCK_MONOTONIC, &m1) == 0);
    struct timespec req = { 0, 50000000L };
    assert(nanosleep(&req, &req) == 0);
    assert(clock_gettime(CLOCK_MONOTONIC, &m2) == 0);
    long elapsed = (m2.tv_sec - m1.tv_sec) * 1000000000L + m2.tv_nsec -
                   m1.tv_nsec;
    assert(elapsed >= 45000000L && elapsed < 1000000000L);

    // несуществующий clockid — EINVAL (syscall-путь при отказе vDSO)
    errno = 0;
    assert(clock_gettime((clockid_t)20, &ts) == -1 && errno == EINVAL);

    assert(sleep(0) == 0);

    printf("test_time passed\n");
    return 0;
}
