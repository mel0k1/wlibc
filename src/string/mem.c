#include <string.h>

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    if (d < s) {
        while (n--)
            *d++ = *s++;
    } else if (d > s) {
        while (n) {
            n--;
            d[n] = s[n];
        }
    }
    return dst;
}

void *memset(void *dst, int c, size_t n)
{
    unsigned char *d = dst;
    while (n--)
        *d++ = (unsigned char)c;
    return dst;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a;
    const unsigned char *y = b;
    for (; n; n--, x++, y++) {
        if (*x != *y)
            return (int)*x - (int)*y;
    }
    return 0;
}

void *memchr(const void *s, int c, size_t n)
{
    const unsigned char *p = s;
    unsigned char uc = (unsigned char)c;
    for (; n; n--, p++) {
        if (*p == uc)
            return (void *)p;
    }
    return 0;
}
