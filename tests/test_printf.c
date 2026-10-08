#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    char b[256];

    sprintf(b, "%d", 42);
    assert(!strcmp(b, "42"));
    sprintf(b, "%d", -42);
    assert(!strcmp(b, "-42"));
    sprintf(b, "%5d|%-5d|%05d", 42, 42, 42);
    assert(!strcmp(b, "   42|42   |00042"));
    sprintf(b, "%+d % d", 42, 42);
    assert(!strcmp(b, "+42  42"));
    sprintf(b, "%lld", LLONG_MIN);
    assert(!strcmp(b, "-9223372036854775808"));
    sprintf(b, "%u", 4294967295u);
    assert(!strcmp(b, "4294967295"));
    sprintf(b, "%llu", 18446744073709551615ULL);
    assert(!strcmp(b, "18446744073709551615"));
    sprintf(b, "%x|%X|%#x|%#o", 48879, 48879, 48879, 8);
    assert(!strcmp(b, "beef|BEEF|0xbeef|010"));
    sprintf(b, "%c%c", 'w', 'l');
    assert(!strcmp(b, "wl"));
    sprintf(b, "%s|%10s|%-10s|%.3s", "ab", "ab", "ab", "abcdef");
    assert(!strcmp(b, "ab|        ab|ab        |abc"));
    sprintf(b, "%p", (void *)0);
    assert(!strcmp(b, "0x0"));
    sprintf(b, "%zu", (size_t)123);
    assert(!strcmp(b, "123"));
    sprintf(b, "%hhd", 300);
    assert(!strcmp(b, "44"));
    sprintf(b, "100%%");
    assert(!strcmp(b, "100%"));

    sprintf(b, "%f", 3.14159);
    assert(!strcmp(b, "3.141590"));
    sprintf(b, "%.2f", 0.125);
    assert(!strcmp(b, "0.13"));
    sprintf(b, "%.0f", 2.5);
    assert(!strcmp(b, "3"));
    sprintf(b, "%+.1f", 2.25);
    assert(!strcmp(b, "+2.3"));
    sprintf(b, "%f", -0.5);
    assert(!strcmp(b, "-0.500000"));
    sprintf(b, "%f", 1000000000000000000.0);
    assert(!strcmp(b, "1000000000000000000.000000"));
    sprintf(b, "%08.2f", 3.14);
    assert(!strcmp(b, "00003.14"));

    assert(printf("") == 0);
    int r = snprintf(NULL, 0, "%d", 12345);
    assert(r == 5);
    char small[4];
    int rn = snprintf(small, sizeof(small), "abcdef");
    assert(rn == 6 && !strcmp(small, "abc") && small[3] == 0);

    printf("test_printf passed\n");
    return 0;
}
