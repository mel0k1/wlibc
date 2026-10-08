#include <string.h>

size_t strlen(const char *s)
{
    const char *p = s;
    while (*p)
        p++;
    return (size_t)(p - s);
}

size_t strnlen(const char *s, size_t n)
{
    size_t i = 0;
    while (i < n && s[i])
        i++;
    return i;
}

char *strcpy(char *dst, const char *src)
{
    char *d = dst;
    while ((*d++ = *src++))
        ;
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
    char *d = dst;
    while (n--) {
        if ((*d++ = *src++) == 0) {
            while (n--)
                *d++ = 0;
            break;
        }
    }
    return dst;
}

char *strcat(char *dst, const char *src)
{
    char *d = dst;
    while (*d)
        d++;
    while ((*d++ = *src++))
        ;
    return dst;
}

char *strncat(char *dst, const char *src, size_t n)
{
    char *d = dst;
    while (*d)
        d++;
    while (n && *src) {
        *d++ = *src++;
        n--;
    }
    *d = 0;
    return dst;
}

int strcmp(const char *a, const char *b)
{
    const unsigned char *x = (const unsigned char *)a;
    const unsigned char *y = (const unsigned char *)b;
    while (*x && *x == *y) {
        x++;
        y++;
    }
    return (int)*x - (int)*y;
}

int strncmp(const char *a, const char *b, size_t n)
{
    const unsigned char *x = (const unsigned char *)a;
    const unsigned char *y = (const unsigned char *)b;
    while (n && *x && *x == *y) {
        x++;
        y++;
        n--;
    }
    return n ? (int)*x - (int)*y : 0;
}

char *strchr(const char *s, int c)
{
    char uc = (char)c;
    for (;; s++) {
        if (*s == uc)
            return (char *)s;
        if (!*s)
            return 0;
    }
}

char *strrchr(const char *s, int c)
{
    const char *last = 0;
    char uc = (char)c;
    for (;; s++) {
        if (*s == uc)
            last = s;
        if (!*s)
            return (char *)last;
    }
}

char *strstr(const char *h, const char *n)
{
    if (!*n)
        return (char *)h;
    for (; *h; h++) {
        const char *a = h;
        const char *b = n;
        while (*a && *b && *a == *b) {
            a++;
            b++;
        }
        if (!*b)
            return (char *)h;
    }
    return 0;
}
