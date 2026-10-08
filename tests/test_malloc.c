#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    void *p0 = malloc(0);
    assert(p0 != NULL);
    free(p0);

    char *a = malloc(100);
    assert(a != NULL);
    memset(a, 7, 100);
    for (int i = 0; i < 100; i++)
        assert(a[i] == 7);

    char *v[64];
    for (int i = 0; i < 64; i++) {
        v[i] = malloc(1 + (i * 37) % 500);
        assert(v[i] != NULL);
        v[i][0] = (char)i;
        v[i][(i * 37) % 500] = (char)(i + 1);
    }
    for (int i = 0; i < 64; i += 2)
        free(v[i]);
    for (int i = 1; i < 64; i += 2) {
        assert(v[i][0] == (char)i);
        assert(v[i][(i * 37) % 500] == (char)(i + 1));
        free(v[i]);
    }
    for (int i = 0; i < 64; i++) {
        v[i] = malloc(1 + (i * 53) % 700);
        assert(v[i] != NULL);
        free(v[i]);
    }

    void *big = malloc(1 << 20);
    assert(big != NULL);
    memset(big, 0xaa, 1 << 20);
    free(big);

    char *r = malloc(10);
    assert(r != NULL);
    memset(r, 'a', 10);
    r = realloc(r, 4096);
    assert(r != NULL);
    for (int i = 0; i < 10; i++)
        assert(r[i] == 'a');
    free(r);

    int *c = calloc(32, sizeof(int));
    assert(c != NULL);
    for (int i = 0; i < 32; i++)
        assert(c[i] == 0);
    free(c);

    free(NULL);

    printf("test_malloc passed\n");
    return 0;
}
