#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static int cmp_str(const void *a, const void *b)
{
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

int main(void)
{
    assert(atoi("  -123abc") == -123);
    assert(atoi("+7") == 7);
    assert(atol("2147483648") == 2147483648L);

    char *end;
    assert(strtol("0x1f", &end, 0) == 31 && *end == 0);
    assert(strtol("010", &end, 0) == 8);
    assert(strtol("42", &end, 0) == 42);
    assert(strtol("zz", &end, 36) == 35 * 36 + 35);
    assert(strtol("42abc", &end, 10) == 42 && *end == 'a');

    errno = 0;
    assert(strtol("99999999999999999999999", &end, 10) == LONG_MAX);
    assert(errno == ERANGE);
    errno = 0;
    assert(strtol("-99999999999999999999999", &end, 10) == LONG_MIN);
    assert(errno == ERANGE);

    int arr[] = { 5, 3, 9, 1, 7, 3, 0 };
    qsort(arr, 7, sizeof(int), cmp_int);
    int want[] = { 0, 1, 3, 3, 5, 7, 9 };
    assert(memcmp(arr, want, sizeof(arr)) == 0);

    int key = 7;
    assert(*(int *)bsearch(&key, arr, 7, sizeof(int), cmp_int) == 7);
    key = 4;
    assert(bsearch(&key, arr, 7, sizeof(int), cmp_int) == NULL);

    const char *strs[] = { "pear", "apple", "fig" };
    qsort(strs, 3, sizeof(char *), cmp_str);
    assert(!strcmp(strs[0], "apple") && !strcmp(strs[1], "fig") &&
           !strcmp(strs[2], "pear"));

    srand(42);
    for (int i = 0; i < 100; i++) {
        int r = rand();
        assert(r >= 0 && r <= RAND_MAX);
    }

    assert(abs(-5) == 5 && abs(5) == 5 && labs(-100L) == 100L);

    printf("test_stdlib passed\n");
    return 0;
}
