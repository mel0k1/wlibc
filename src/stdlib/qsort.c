#include <stdlib.h>
#include <string.h>

static void swap(char *a, char *b, size_t w)
{
    while (w--) {
        char t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

static void sift(char *base, size_t n, size_t w, int (*cmp)(const void *, const void *),
                 size_t i)
{
    for (;;) {
        size_t l = 2 * i + 1;
        size_t r = l + 1;
        size_t m = i;
        if (l < n && cmp(base + l * w, base + m * w) > 0)
            m = l;
        if (r < n && cmp(base + r * w, base + m * w) > 0)
            m = r;
        if (m == i)
            return;
        swap(base + i * w, base + m * w, w);
        i = m;
    }
}

void qsort(void *base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *))
{
    char *b = base;
    if (nmemb < 2)
        return;
    for (size_t i = nmemb / 2; i-- > 0;)
        sift(b, nmemb, size, cmp, i);
    for (size_t e = nmemb - 1; e > 0; e--) {
        swap(b, b + e * size, size);
        sift(b, e, size, cmp, 0);
    }
}

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*cmp)(const void *, const void *))
{
    const char *b = base;
    while (nmemb) {
        size_t mid = nmemb / 2;
        int r = cmp(key, b + mid * size);
        if (r == 0)
            return (void *)(b + mid * size);
        if (r < 0) {
            nmemb = mid;
        } else {
            b += (mid + 1) * size;
            nmemb -= mid + 1;
        }
    }
    return 0;
}
