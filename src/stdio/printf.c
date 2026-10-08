#include <stdarg.h>
#include <stdio.h>

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vdprintf(1, fmt, ap);
    va_end(ap);
    return r;
}

int vprintf(const char *fmt, va_list ap)
{
    return vdprintf(1, fmt, ap);
}
