#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

long strtol(const char *s, char **end, int base)
{
    const char *p = s;
    while (isspace(*p))
        p++;
    int neg = 0;
    if (*p == '+' || *p == '-') {
        neg = (*p == '-');
        p++;
    }
    if ((!base || base == 16) && p[0] == '0' && (p[1] == 'x' || p[1] == 'X') &&
        isxdigit(p[2])) {
        p += 2;
        base = 16;
    } else if (!base && p[0] == '0') {
        base = 8;
    } else if (!base) {
        base = 10;
    }
    unsigned long cutoff = neg ? (unsigned long)LONG_MAX + 1UL : (unsigned long)LONG_MAX;
    unsigned long acc = 0;
    int any = 0, overflow = 0;
    for (;; p++) {
        int d;
        if (*p >= '0' && *p <= '9')
            d = *p - '0';
        else if (*p >= 'a' && *p <= 'z')
            d = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'Z')
            d = *p - 'A' + 10;
        else
            break;
        if (d >= base)
            break;
        any = 1;
        if (acc > (cutoff - (unsigned long)d) / (unsigned long)base)
            overflow = 1;
        else
            acc = acc * (unsigned long)base + (unsigned long)d;
    }
    if (end)
        *end = (char *)(any ? p : s);
    if (overflow) {
        errno = ERANGE;
        return neg ? LONG_MIN : LONG_MAX;
    }
    return neg ? -(long)acc : (long)acc;
}

int atoi(const char *s)
{
    return (int)strtol(s, 0, 10);
}

long atol(const char *s)
{
    return strtol(s, 0, 10);
}
