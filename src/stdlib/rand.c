#include <stdlib.h>

static unsigned long long rs = 1;

void srand(unsigned seed)
{
    rs = seed ? seed : 1;
}

int rand(void)
{
    rs = rs * 6364136223846793005ULL + 1442695040888963407ULL;
    return (int)((rs >> 33) & 0x7fffffff);
}
