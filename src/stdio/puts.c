#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libc.h>

int puts(const char *s)
{
    size_t n = strlen(s);
    if (__fd_write_all(STDOUT_FILENO, s, n) < 0)
        return EOF;
    if (__fd_write_all(STDOUT_FILENO, "\n", 1) < 0)
        return EOF;
    return (int)(n + 1);
}

int putchar(int c)
{
    unsigned char ch = (unsigned char)c;
    if (write(STDOUT_FILENO, &ch, 1) == 1)
        return ch;
    return EOF;
}
