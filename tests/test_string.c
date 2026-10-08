#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char b[64];

    assert(strlen("wlibc") == 5);

    strcpy(b, "hello");
    assert(strcmp(b, "hello") == 0);
    strcat(b, " world");
    assert(strcmp(b, "hello world") == 0);
    assert(strncmp("hello world", "hello", 5) == 0);

    strncpy(b, "abc", 6);
    assert(b[3] == 0 && b[4] == 0 && b[5] == 0);
    assert(strncmp(b, "abc", 3) == 0);

    memset(b, 'x', 8);
    b[8] = 0;
    assert(strcmp(b, "xxxxxxxx") == 0);
    assert(memcmp("abc\0z", "abc\0y", 4) == 0);
    assert(memcmp("abc\0z", "abc\0y", 5) > 0);

    strcpy(b, "0123456789");
    memmove(b + 2, b, 8);
    assert(memcmp(b, "0101234567", 10) == 0);
    memmove(b, b + 2, 8);
    assert(memcmp(b, "01234567", 8) == 0);

    assert(strchr("hello", 'l') - "hello" == 2);
    assert(strchr("hello", 0) - "hello" == 5);
    assert(strchr("hello", 'z') == NULL);
    assert(strrchr("hello", 'l') - "hello" == 3);
    assert(strrchr("hello", 'z') == NULL);

    assert(strcmp(strstr("ababc", "abc"), "abc") == 0);
    assert(strstr("ababc", "zz") == NULL);
    assert(strstr("ab", "") != NULL);
    assert(strcmp(strstr("ab", ""), "ab") == 0);

    char *d = strdup("dup");
    assert(d && strcmp(d, "dup") == 0);
    free(d);

    assert(strncat(strcpy(b, "hi"), " world!", 3) == b);
    assert(strcmp(b, "hi wo") == 0);

    printf("test_string passed\n");
    return 0;
}
