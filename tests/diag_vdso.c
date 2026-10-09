#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include <time.h>
#include <vdso.h>

static unsigned long hex(const char *p)
{
    unsigned long v = 0;
    for (;; p++) {
        int c = *p;
        if (c >= '0' && c <= '9')
            v = v * 16 + (unsigned)(c - '0');
        else if (c >= 'a' && c <= 'f')
            v = v * 16 + (unsigned)(c - 'a' + 10);
        else
            break;
    }
    return v;
}

int main(void)
{
    fprintf(stderr, "active=%d\n", __vdso_active);
    fprintf(stderr, "f=%lx getres=%lx gettimeofday=%lx time=%lx\n",
            (unsigned long)__vdso_clock_gettime_f,
            (unsigned long)__vdso_clock_getres_f,
            (unsigned long)__vdso_gettimeofday_f,
            (unsigned long)__vdso_time_f);

    // диапазон маппинга [vdso]
    FILE *f = fopen("/proc/self/maps", "r");
    char line[512];
    unsigned long lo = 0, hi = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "[vdso]")) {
            lo = hex(line);
            hi = hex(strchr(line, '-') + 1);
            break;
        }
    }
    fclose(f);
    fprintf(stderr, "vdso map: %lx-%lx\n", lo, hi);

    // прямой вызов vDSO для clockid 20
    struct timespec ts;
    long vr = __vdso_clock_gettime_f(20, &ts);
    fprintf(stderr, "vdso(20) raw=%lx (%ld)\n", (unsigned long)vr, vr);

    // наша обёртка
    errno = 0;
    int r = clock_gettime((clockid_t)20, &ts);
    fprintf(stderr, "clock_gettime(20)=%d errno=%d\n", r, errno);

    // сырой syscall
    errno = 0;
    long raw = __syscall2(SYS_clock_gettime, 20, (long)&ts);
    fprintf(stderr, "raw syscall=%ld errno=%d\n", raw, errno);

    // валидные часы через vDSO напрямую
    vr = __vdso_clock_gettime_f(0, &ts);
    fprintf(stderr, "vdso(REALTIME)=%ld sec=%ld\n", vr, ts.tv_sec);
    return 0;
}
