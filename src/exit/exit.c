#include <stdlib.h>
#include <unistd.h>
#include <libc.h>

#define ATEXIT_MAX 32

static void (*atf[ATEXIT_MAX])(void);
static unsigned atn;

int atexit(void (*func)(void))
{
    if (atn == ATEXIT_MAX)
        return -1;
    atf[atn++] = func;
    return 0;
}

void exit(int code)
{
    while (atn)
        atf[--atn]();
    __stdio_exit();
    _exit(code);
}
