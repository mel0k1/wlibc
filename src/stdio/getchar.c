#include <stdio.h>
#include <unistd.h>

int getchar(void)
{
    unsigned char ch;
    ssize_t r = read(STDIN_FILENO, &ch, 1);
    if (r == 1)
        return ch;
    return EOF;
}
