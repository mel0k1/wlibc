#include <math.h>

// исключения FP не контролируются, ведёт себя как rint
double nearbyint(double x)
{
    return rint(x);
}
