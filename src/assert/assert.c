#include <stdio.h>
#include <stdlib.h>

void __assert_fail(const char *expr, const char *file, int line, const char *func)
{
    dprintf(2, "%s: %d: %s: Assertion `%s` failed.\n", file, line, func, expr);
    abort();
}
