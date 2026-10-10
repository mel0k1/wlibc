#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
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

    // несуществующий clockid — отказ vDSO и ошибка через syscall-путь;
    // конкретный errno зависит от ядра (EINVAL/ENODEV)
    errno = 0;
    assert(clock_gettime((clockid_t)20, &ts) == -1 && errno != 0);

    assert(sleep(0) == 0);

    // календарь: эпоха
    time_t te = 0;
    struct tm ge;
    gmtime_r(&te, &ge);
    assert(ge.tm_year == 70 && ge.tm_mon == 0 && ge.tm_mday == 1);
    assert(ge.tm_hour == 0 && ge.tm_min == 0 && ge.tm_sec == 0);
    assert(ge.tm_wday == 4 && ge.tm_yday == 0);
    assert(!strcmp(asctime(&ge), "Thu Jan  1 00:00:00 1970\n"));
    assert(!strcmp(ctime(&te), "Thu Jan  1 00:00:00 1970\n"));

    // 2009-02-13 23:31:30 UTC — пятница
    time_t tf = 1234567890;
    struct tm gf;
    gmtime_r(&tf, &gf);
    assert(gf.tm_year == 109 && gf.tm_mon == 1 && gf.tm_mday == 13);
    assert(gf.tm_hour == 23 && gf.tm_min == 31 && gf.tm_sec == 30);
    assert(gf.tm_wday == 5 && gf.tm_yday == 43);

    // отрицательное время
    time_t tn = -86400;
    struct tm gn;
    gmtime_r(&tn, &gn);
    assert(gn.tm_year == 69 && gn.tm_mon == 11 && gn.tm_mday == 31);
    assert(gn.tm_wday == 3);

    // високосный день: 2000-02-29 00:00:00 UTC
    time_t tleap = 951782400;
    struct tm glp;
    gmtime_r(&tleap, &glp);
    assert(glp.tm_mday == 29 && glp.tm_mon == 1 && glp.tm_yday == 59);
    assert(glp.tm_wday == 2);

    // mktime/timegm нормализует поля вне допустимых диапазонов
    struct tm x;
    memset(&x, 0, sizeof x);
    x.tm_year = 100;
    x.tm_mon = 12; // январь 2001
    x.tm_mday = 1;
    assert(timegm(&x) == 978307200);
    assert(x.tm_year == 101 && x.tm_mon == 0 && x.tm_wday == 1);
    x.tm_mday = 0; // 2001-01-00 — последний день декабря
    assert(timegm(&x) == 978307200 - 86400);
    x.tm_year = 101; // 2001-01, месяц -1 → декабрь 2000
    x.tm_mon = -1;
    x.tm_mday = 1;
    x.tm_hour = 12;
    assert(timegm(&x) == 975628800 + 43200);
    assert(x.tm_year == 100 && x.tm_mon == 11 && x.tm_hour == 12);

    // mktime = timegm: локальная зона всегда UTC
    struct tm mk;
    memset(&mk, 0, sizeof mk);
    mk.tm_year = 70;
    mk.tm_mday = 1;
    assert(mktime(&mk) == 0);
    assert(mk.tm_wday == 4);

    // localtime совпадает с gmtime
    struct tm gl;
    localtime_r(&tf, &gl);
    assert(gl.tm_hour == gf.tm_hour && gl.tm_wday == gf.tm_wday);

    // roundtrip туда-обратно, положительные и отрицательные
    for (long long u = 1; u < 4000000000LL; u = u * 3 + 7) {
        struct tm g2;
        time_t src = (time_t)u;
        gmtime_r(&src, &g2);
        assert(timegm(&g2) == src);
    }
    for (long long u = -1; u > -4000000000LL; u = u * 3 - 7) {
        struct tm g2;
        time_t src = (time_t)u;
        gmtime_r(&src, &g2);
        assert(timegm(&g2) == src);
    }

    assert(difftime(100, 40) == 60.0);

    // strftime
    char sb[64];
    size_t sn = strftime(sb, sizeof sb, "%Y-%m-%d %H:%M:%S", &ge);
    assert(sn == 19 && !strcmp(sb, "1970-01-01 00:00:00"));
    sn = strftime(sb, sizeof sb, "%a %b %e %j %p %Z %z %%", &gf);
    assert(sn == 29 && !strcmp(sb, "Fri Feb 13 044 PM UTC +0000 %"));
    sn = strftime(sb, sizeof sb, "%U %W %C %y %u %w %F %T %R %D", &gf);
    assert(!strcmp(sb, "06 06 20 09 5 5 2009-02-13 23:31:30 23:31 02/13/09"));
    assert(strftime(sb, 8, "%Y-%m-%d", &ge) == 0); // не влезло
    assert(strftime(sb, 11, "%Y-%m-%d", &ge) == 10);

    printf("test_time passed\n");
    return 0;
}
