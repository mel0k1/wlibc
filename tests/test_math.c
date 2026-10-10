#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "math_ref.h"

#define TBL_ULP 4

static unsigned long long db(double d)
{
    union {
        double f;
        unsigned long long i;
    } u = { d };
    return u.i;
}

static double bd(unsigned long long i)
{
    // первым членом делаем i, иначе { i } инициализирует f
    union {
        unsigned long long i;
        double f;
    } u = { i };
    return u.f;
}

static int fails;

static void fail(const char *what, unsigned row, int fn, double x, double y,
                 double got, double want)
{
    // %g нет в wlibc vfprintf — печатаем биты
    printf("FAIL %s row=%u fn=%d x=%016llx y=%016llx got=%016llx want=%016llx rb=%016llx\n",
           what, row, fn, db(x), db(y), db(got), db(want), 0ULL);
    fails++;
}

// эталон в ulp; для NaN/знаков нуля — отдельные правила
static int dbl_ok(double got, double want)
{
    if (db(got) == db(want))
        return 1;
    if (isnan(want))
        return isnan(got);
    if (isnan(got))
        return 0;
    if ((db(got) >> 63) != (db(want) >> 63))
        return got == 0.0 && want == 0.0;
    long long a = (long long)db(got), b = (long long)db(want);
    return (a > b ? a - b : b - a) <= TBL_ULP;
}

static double call_d(int fn, double x, double y)
{
    switch (fn) {
    case F_ACOS: return acos(x);
    case F_ASIN: return asin(x);
    case F_ATAN: return atan(x);
    case F_ATAN2: return atan2(x, y);
    case F_COS: return cos(x);
    case F_SIN: return sin(x);
    case F_TAN: return tan(x);
    case F_ACOSH: return acosh(x);
    case F_ASINH: return asinh(x);
    case F_ATANH: return atanh(x);
    case F_COSH: return cosh(x);
    case F_SINH: return sinh(x);
    case F_TANH: return tanh(x);
    case F_EXP: return exp(x);
    case F_EXP2: return exp2(x);
    case F_EXPM1: return expm1(x);
    case F_LOG: return log(x);
    case F_LOG2: return log2(x);
    case F_LOG10: return log10(x);
    case F_LOG1P: return log1p(x);
    case F_POW: return pow(x, y);
    case F_CBRT: return cbrt(x);
    case F_HYPOT: return hypot(x, y);
    case F_SQRT: return sqrt(x);
    case F_FMOD: return fmod(x, y);
    case F_FABS: return fabs(x);
    case F_FLOOR: return floor(x);
    case F_CEIL: return ceil(x);
    case F_TRUNC: return trunc(x);
    case F_ROUND: return round(x);
    case F_LDEXP: return ldexp(x, (int)y);
    case F_SCALBN: return scalbn(x, (int)y);
    case F_ILOGB: return (double)ilogb(x);
    case F_LOGB: return logb(x);
    case F_FMAX: return fmax(x, y);
    case F_FMIN: return fmin(x, y);
    case F_FDIM: return fdim(x, y);
    case F_COPYSIGN: return copysign(x, y);
    }
    return 0;
}

static float call_f(int fn, float x, float y)
{
    switch (fn) {
    case F_ACOS: return acosf(x);
    case F_ASIN: return asinf(x);
    case F_ATAN: return atanf(x);
    case F_ATAN2: return atan2f(x, y);
    case F_COS: return cosf(x);
    case F_SIN: return sinf(x);
    case F_TAN: return tanf(x);
    case F_COSH: return coshf(x);
    case F_SINH: return sinhf(x);
    case F_TANH: return tanhf(x);
    case F_EXP: return expf(x);
    case F_EXP2: return exp2f(x);
    case F_EXPM1: return expm1f(x);
    case F_LOG: return logf(x);
    case F_LOG2: return log2f(x);
    case F_LOG10: return log10f(x);
    case F_LOG1P: return log1pf(x);
    case F_POW: return powf(x, y);
    case F_CBRT: return cbrtf(x);
    case F_HYPOT: return hypotf(x, y);
    case F_SQRT: return sqrtf(x);
    case F_FMOD: return fmodf(x, y);
    case F_FABS: return fabsf(x);
    case F_FLOOR: return floorf(x);
    case F_CEIL: return ceilf(x);
    case F_TRUNC: return truncf(x);
    case F_ROUND: return roundf(x);
    case F_FMAX: return fmaxf(x, y);
    case F_FMIN: return fminf(x, y);
    case F_FDIM: return fdimf(x, y);
    case F_COPYSIGN: return copysignf(x, y);
    }
    return 0;
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    // эталонная таблица double
    for (unsigned i = 0; i < sizeof math_ref_d / sizeof math_ref_d[0]; i++) {
        const struct math_ref *e = &math_ref_d[i];
        double x = bd(e->xb), y = bd(e->yb), want = bd(e->rb);
        double got = call_d(e->fn, x, y);
        int zero_pair = want == 0.0 && got == 0.0 &&
                        (e->fn == F_FMAX || e->fn == F_FMIN);
        if (e->exact) {
            if (isnan(want) ? !isnan(got)
                            : !zero_pair && db(got) != db(want))
                fail("exact", i, e->fn, x, y, got, want);
        } else if (!zero_pair && !dbl_ok(got, want)) {
            // fmax/fmin на паре нулей со разными знаками — по IEEE
            // допустим любой знак результата
            int zeros = want == 0.0 && got == 0.0 &&
                        (e->fn == F_FMAX || e->fn == F_FMIN);
            if (!zeros)
                fail("ulp", i, e->fn, x, y, got, want);
        }
    }

    // эталонная таблица float
    for (unsigned i = 0; i < sizeof math_ref_f / sizeof math_ref_f[0]; i++) {
        const struct math_ref *e = &math_ref_f[i];
        float x = (float)bd(e->xb), y = (float)bd(e->yb);
        union {
            float f;
            unsigned u;
        } w = { 0 };
        w.u = (unsigned)e->rb;
        float got = call_f(e->fn, x, y);
        union {
            unsigned u;
            float f;
        } g;
        g.f = got;
        int want_nan = (w.u & 0x7f800000u) == 0x7f800000u &&
                       (w.u & 0x7fffffu);
        int got_nan = (g.u & 0x7f800000u) == 0x7f800000u &&
                      (g.u & 0x7fffffu);
        int zero_pair_f = w.f == 0.0f && got == 0.0f &&
                          (e->fn == F_FMAX || e->fn == F_FMIN);
        if (want_nan ? !got_nan : !zero_pair_f && g.u != w.u) {
            if (e->exact || want_nan) {
                printf("FAIL fexact fn=%d got=%08x want=%08x\n", e->fn, g.u,
                       w.u);
                fails++;
            } else {
                // ulp для float (не inf/nan)
                int sign_g = g.u >> 31, sign_w = w.u >> 31;
                if (sign_g == sign_w &&
                    (g.u & 0x7f800000u) != 0x7f800000u &&
                    (w.u & 0x7f800000u) != 0x7f800000u) {
                    int a = (int)g.u, b = (int)w.u;
                    if ((a > b ? a - b : b - a) > TBL_ULP) {
                        printf("FAIL fulp fn=%d got=%08x want=%08x\n", e->fn,
                               g.u, w.u);
                        fails++;
                    }
                } else if (!(got == 0.0f && w.f == 0.0f)) {
                    printf("FAIL fulp2 fn=%d got=%08x want=%08x\n", e->fn,
                           g.u, w.u);
                    fails++;
                }
            }
        }
    }

    // --- точечные проверки ---

    // modf
    double ip;
    assert(fabs(modf(12345.6789, &ip) - 0.6789) < 1e-12 && ip == 12345.0);
    assert(modf(-0.75, &ip) == -0.75 && ip == -0.0);
    assert(modf(0.0, &ip) == 0.0 && ip == 0.0);
    assert(db(modf(-0.0, &ip)) == db(-0.0));

    // frexp/ldexp/scalbn
    int e;
    assert(frexp(8.0, &e) == 0.5 && e == 4);
    assert(frexp(-3.0, &e) == -0.75 && e == 2);
    assert(frexp(0.0, &e) == 0.0 && e == 0);
    assert(ldexp(1.0, 10) == 1024.0);
    assert(ldexp(3.0, -2) == 0.75);
    assert(ldexp(1.5, 0) == 1.5);
    assert(db(ldexp(1.0, -1074)) == db(DBL_TRUE_MIN));
    assert(isinf(ldexp(1.0, 1024)));
    assert(scalbn(1.5, 4) == 24.0);

    // ilogb/logb
    assert(ilogb(1.0) == 0 && ilogb(8.0) == 3 && ilogb(0.5) == -1);
    assert(ilogb(0.0) == FP_ILOGB0);
    assert(ilogb(NAN) == FP_ILOGBNAN);
    assert(ilogb(INFINITY) == INT_MAX);
    assert(logb(8.0) == 3.0 && logb(0.5) == -1.0);
    assert(isinf(logb(0.0)) && logb(0.0) < 0);
    assert(isinf(logb(INFINITY)));

    // классификация
    assert(isnan(NAN) && !isnan(1.0));
    assert(isinf(INFINITY) && isinf(-INFINITY) && !isinf(1.0));
    assert(isfinite(1.0) && isfinite(-0.0) && !isfinite(INFINITY));
    assert(fpclassify(0.0) == FP_ZERO && fpclassify(-0.0) == FP_ZERO);
    assert(fpclassify(INFINITY) == FP_INFINITE);
    assert(fpclassify(NAN) == FP_NAN);
    assert(fpclassify(DBL_MIN) == FP_NORMAL);
    assert(fpclassify(1e-310) == FP_SUBNORMAL);
    assert(signbit(-0.0) && !signbit(0.0) && signbit(-1.0));

    // нули и константы
    assert(db(sqrt(-0.0)) == db(-0.0));
    assert(isnan(sqrt(-1.0)) && isnan(log(-1.0)));
    assert(isinf(log(-0.0)) && log(-0.0) < 0);
    assert(exp(0.0) == 1.0 && exp(-0.0) == 1.0);
    assert(log(1.0) == 0.0);
    assert(isinf(exp(INFINITY)) && exp(-INFINITY) == 0.0);
    assert(isinf(HUGE_VAL) && isinf(HUGE_VALF));
    assert(isnan(nan("")) && isnan(NAN));

    // rint/nearbyint/round/ceil/floor/trunc
    assert(rint(0.5) == 0.0 && rint(1.5) == 2.0 && rint(2.5) == 2.0);
    assert(rint(-0.5) == -0.0 && rint(-1.5) == -2.0);
    assert(rint(2.49) == 2.0 && rint(-2.49) == -2.0);
    assert(nearbyint(1.5) == 2.0);
    assert(round(0.5) == 1.0 && round(2.5) == 3.0 && round(-2.5) == -3.0);
    assert(round(0.49) == 0.0);
    assert(floor(-1.1) == -2.0 && floor(1.9) == 1.0);
    assert(ceil(1.1) == 2.0 && ceil(-1.9) == -1.0);
    assert(trunc(-1.9) == -1.0 && trunc(1.9) == 1.0);
    assert(db(trunc(-0.5)) == db(-0.0));

    // базовые тождества
    assert(sqrt(4.0) == 2.0 && sqrt(2.25) == 1.5);
    double dx;
    for (dx = 0.1; dx < 50.0; dx *= 3.7) {
        double r = sqrt(dx);
        assert(fabs(r * r - dx) <= 1e-12 * dx);
        double s = sin(dx), c = cos(dx);
        assert(fabs(s * s + c * c - 1.0) < 1e-12);
        assert(fabs(exp(log(dx)) - dx) <= 1e-12 * dx);
        assert(fabs(log(exp(dx)) - dx) <= 1e-12 * (1.0 + dx));
        assert(fabs(pow(dx, 2.0) - dx * dx) <= 1e-11 * (dx * dx));
        assert(fabs(cbrt(dx * dx * dx) - dx) <= 1e-11 * dx);
        assert(fabs(tan(dx) - s / c) <= 1e-9 * fabs(s / c) + 1e-12);
    }

    // специальные значения
    assert(pow(0.0, 0.0) == 1.0);
    assert(pow(-1.0, 3.0) == -1.0);
    assert(fabs(pow(2.0, 10.0) - 1024.0) < 1e-9);
    assert(fabs(pow(4.0, 0.5) - 2.0) < 1e-15);
    assert(isnan(pow(-2.0, 0.5)));
    assert(pow(1.0, INFINITY) == 1.0);
    assert(hypot(3.0, 4.0) == 5.0 && hypot(5.0, 12.0) == 13.0);
    assert(hypot(INFINITY, 1.0) == INFINITY);
    assert(fabs(cbrt(27.0) - 3.0) < 1e-14 && fabs(cbrt(-8.0) + 2.0) < 1e-14);
    assert(tanh(0.0) == 0.0 && tanh(INFINITY) == 1.0 && tanh(-INFINITY) == -1.0);
    assert(sinh(INFINITY) == INFINITY && cosh(-INFINITY) == INFINITY);
    assert(isinf(atanh(1.0)) && isinf(atanh(-1.0)));
    assert(atan2(0.0, 0.0) == 0.0);
    assert(fabs(atan2(0.0, -1.0) - M_PI) < 1e-15);
    assert(fabs(atan2(INFINITY, INFINITY) - M_PI_4) < 1e-15);
    assert(fabs(log2(8.0) - 3.0) < 1e-14 && fabs(log10(100.0) - 2.0) < 1e-14);
    assert(fabs(exp2(10.0) - 1024.0) < 1e-12);
    assert(expm1(0.0) == 0.0 && log1p(0.0) == 0.0);
    assert(fabs(expm1(1e-7) - 1.00000005e-7) < 1e-16);
    assert(fmax(1.0, NAN) == 1.0 && fmax(NAN, 1.0) == 1.0);
    assert(fmin(-1.0, NAN) == -1.0 && fmin(NAN, -1.0) == -1.0);
    assert(db(fmax(0.0, -0.0)) == db(0.0));
    assert(db(fmin(0.0, -0.0)) == db(-0.0));
    assert(fdim(5.0, 3.0) == 2.0 && fdim(3.0, 5.0) == 0.0);
    assert(isnan(fdim(NAN, 1.0)));
    assert(db(copysign(3.0, -0.0)) == db(-3.0));

    // float: точки
    assert(sqrtf(4.0f) == 2.0f && fabsf(-2.5f) == 2.5f);
    assert(expf(0.0f) == 1.0f && logf(1.0f) == 0.0f);
    assert(floorf(-1.5f) == -2.0f && ceilf(1.5f) == 2.0f);
    assert(isnan(logf(-1.0f)));
    assert(isinf(expf(1000.0f)) && expf(-1000.0f) == 0.0f);
    assert(fmaxf(1.0f, NAN) == 1.0f);
    assert(fpclassify(1e-45f) == FP_SUBNORMAL);

    if (fails) {
        printf("test_math: %d FAILURES\n", fails);
        return 1;
    }
    printf("test_math passed\n");
    return 0;
}
