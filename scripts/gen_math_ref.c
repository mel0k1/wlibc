// генератор эталонной таблицы для tests/test_math.c
// собирается СИСТЕМНЫМ gcc с glibc libm: gcc -O0 -fno-builtin gen_math_ref.c -lm
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

enum {
    F_ACOS, F_ASIN, F_ATAN, F_ATAN2, F_COS, F_SIN, F_TAN,
    F_ACOSH, F_ASINH, F_ATANH, F_COSH, F_SINH, F_TANH,
    F_EXP, F_EXP2, F_EXPM1, F_LOG, F_LOG2, F_LOG10, F_LOG1P,
    F_POW, F_CBRT, F_HYPOT, F_SQRT, F_FMOD,
    F_FABS, F_FLOOR, F_CEIL, F_TRUNC, F_ROUND,
    F_FREXP, F_LDEXP, F_SCALBN, F_MODF, F_ILOGB, F_LOGB,
    F_FMAX, F_FMIN, F_FDIM, F_COPYSIGN,
};

static const double xs[] = {
    0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, -2.0, 0.25,
    3.14159265358979, -3.14159265358979, 1.5707963267948966,
    0.7853981633974483, 2.718281828459045, 0.6931471805599453,
    10.0, -10.0, 100.0, 1e10, 1e-10, 1e100, 1e-100,
    12345.6789, -12345.6789, 0.9999999999, 1.0000000001,
    4294967296.5, 3.7e-5, -3.7e-5, 1e15, 1e-15, 123.456e-9,
};

static const double ys[] = {
    1.0, 2.0, 3.0, 0.5, -0.5, 3.14159265358979, 10.0, -10.0,
    1e10, 1e-10, 0.0, -1.0, 123.456, -0.75, 2.5, -2.5,
};

#define NX (int)(sizeof xs / sizeof xs[0])
#define NY (int)(sizeof ys / sizeof ys[0])

static uint64_t db(double d)
{
    uint64_t u;
    memcpy(&u, &d, 8);
    return u;
}

static uint32_t fb(float f)
{
    uint32_t u;
    memcpy(&u, &f, 4);
    return u;
}

static int n_entries;

static void emit1(int fn, int exact, double x, double r)
{
    printf("    { %d, %d, 0x%016lxULL, 0, 0x%016lxULL },\n", fn, exact,
           db(x), db(r));
    n_entries++;
}

static void emit2(int fn, int exact, double x, double y, double r)
{
    printf("    { %d, %d, 0x%016lxULL, 0x%016lxULL, 0x%016lxULL },\n", fn,
           exact, db(x), db(y), db(r));
    n_entries++;
}

static void emitf(int fn, int exact, double x, float r)
{
    printf("    { %d, %d, 0x%016lxULL, 0, 0x%08xU },\n", fn, exact, db(x),
           fb(r));
    n_entries++;
}

static void emitf2(int fn, int exact, double x, double y, float r)
{
    printf("    { %d, %d, 0x%016lxULL, 0x%016lxULL, 0x%08xU },\n", fn,
           exact, db(x), db(y), fb(r));
    n_entries++;
}

int main(void)
{
    printf("// сгенерировано scripts/gen_math_ref.c, glibc libm\n");
    printf("#pragma once\n\n");
    printf("enum {\n");
    printf("    F_ACOS, F_ASIN, F_ATAN, F_ATAN2, F_COS, F_SIN, F_TAN,\n");
    printf("    F_ACOSH, F_ASINH, F_ATANH, F_COSH, F_SINH, F_TANH,\n");
    printf("    F_EXP, F_EXP2, F_EXPM1, F_LOG, F_LOG2, F_LOG10, F_LOG1P,\n");
    printf("    F_POW, F_CBRT, F_HYPOT, F_SQRT, F_FMOD,\n");
    printf("    F_FABS, F_FLOOR, F_CEIL, F_TRUNC, F_ROUND,\n");
    printf("    F_FREXP, F_LDEXP, F_SCALBN, F_MODF, F_ILOGB, F_LOGB,\n");
    printf("    F_FMAX, F_FMIN, F_FDIM, F_COPYSIGN,\n");
    printf("};\n\n");
    printf("struct math_ref {\n    int fn;\n    int exact;\n"
           "    unsigned long long xb, yb, rb;\n};\n\n");
    printf("static const struct math_ref math_ref_d[] = {\n");

    for (int i = 0; i < NX; i++) {
        double x = xs[i];

        emit1(F_EXP, 0, x, exp(x));
        emit1(F_EXP2, 0, x, exp2(x));
        emit1(F_EXPM1, 0, x, expm1(x));
        emit1(F_LOG, 0, x, log(x));
        emit1(F_LOG2, 0, x, log2(x));
        emit1(F_LOG10, 0, x, log10(x));
        emit1(F_LOG1P, 0, x, log1p(x));
        emit1(F_CBRT, 0, x, cbrt(x));
        emit1(F_SQRT, 1, x, sqrt(x));
        emit1(F_FABS, 1, x, fabs(x));
        emit1(F_FLOOR, 1, x, floor(x));
        emit1(F_CEIL, 1, x, ceil(x));
        emit1(F_TRUNC, 1, x, trunc(x));
        emit1(F_ROUND, 1, x, round(x));
        emit1(F_SIN, 0, x, sin(x));
        emit1(F_COS, 0, x, cos(x));
        emit1(F_TAN, 0, x, tan(x));
        if (x >= -1.0 && x <= 1.0) {
            emit1(F_ASIN, 0, x, asin(x));
            emit1(F_ACOS, 0, x, acos(x));
        }
        if (x > -1.0 && x < 1.0) {
            emit1(F_ATANH, 0, x, atanh(x));
        }
        if (x >= 1.0) {
            emit1(F_ACOSH, 0, x, acosh(x));
        }
        emit1(F_ASINH, 0, x, asinh(x));
        emit1(F_SINH, 0, x, sinh(x));
        emit1(F_COSH, 0, x, cosh(x));
        if (fabs(x) < 20.0) {
            emit1(F_TANH, 0, x, tanh(x));
        }
        emit1(F_ATAN, 0, x, atan(x));
        emit1(F_LOGB, 1, x, logb(x));
        emit1(F_ILOGB, 1, x, (double)ilogb(x));

        for (int k = -70; k <= 70; k += 35) {
            emit2(F_LDEXP, 1, x, (double)k, ldexp(x, k));
            emit2(F_SCALBN, 1, x, (double)k, scalbn(x, k));
        }

        for (int j = 0; j < NY; j++) {
            double y = ys[j];
            emit2(F_POW, 0, x, y, pow(x, y));
            emit2(F_ATAN2, 0, x, y, atan2(x, y));
            emit2(F_HYPOT, 0, x, y, hypot(x, y));
            emit2(F_FMOD, 1, x, y, fmod(x, y));
            emit2(F_FMAX, 1, x, y, fmax(x, y));
            emit2(F_FMIN, 1, x, y, fmin(x, y));
            emit2(F_FDIM, 1, x, y, fdim(x, y));
            emit2(F_COPYSIGN, 1, x, y, copysign(x, y));
        }
    }
    printf("};\n");

    printf("static const struct math_ref math_ref_f[] = {\n");
    for (int i = 0; i < NX; i++) {
        float x = (float)xs[i];
        emitf(F_EXP, 0, x, expf(x));
        emitf(F_LOG, 0, x, logf(x));
        emitf(F_EXP2, 0, x, exp2f(x));
        emitf(F_LOG2, 0, x, log2f(x));
        emitf(F_EXPM1, 0, x, expm1f(x));
        emitf(F_LOG1P, 0, x, log1pf(x));
        emitf(F_LOG10, 0, x, log10f(x));
        emitf(F_CBRT, 0, x, cbrtf(x));
        emitf(F_SQRT, 1, x, sqrtf(x));
        emitf(F_FABS, 1, x, fabsf(x));
        emitf(F_FLOOR, 1, x, floorf(x));
        emitf(F_CEIL, 1, x, ceilf(x));
        emitf(F_TRUNC, 1, x, truncf(x));
        emitf(F_ROUND, 1, x, roundf(x));
        emitf(F_SIN, 0, x, sinf(x));
        emitf(F_COS, 0, x, cosf(x));
        emitf(F_TAN, 0, x, tanf(x));
        if (x >= -1.0f && x <= 1.0f) {
            emitf(F_ASIN, 0, x, asinf(x));
            emitf(F_ACOS, 0, x, acosf(x));
        }
        emitf(F_ATAN, 0, x, atanf(x));
        emitf(F_SINH, 0, x, sinhf(x));
        emitf(F_COSH, 0, x, coshf(x));
        if (fabsf(x) < 20.0f) {
            emitf(F_TANH, 0, x, tanhf(x));
        }
        for (int j = 0; j < NY; j++) {
            float y = (float)ys[j];
            emitf2(F_POW, 0, x, y, powf(x, y));
            emitf2(F_ATAN2, 0, x, y, atan2f(x, y));
            emitf2(F_HYPOT, 0, x, y, hypotf(x, y));
            emitf2(F_FMOD, 1, x, y, fmodf(x, y));
            emitf2(F_FMAX, 1, x, y, fmaxf(x, y));
            emitf2(F_FMIN, 1, x, y, fminf(x, y));
            emitf2(F_FDIM, 1, x, y, fdimf(x, y));
            emitf2(F_COPYSIGN, 1, x, y, copysignf(x, y));
        }
    }
    printf("};\n");
    fprintf(stderr, "entries: %d\n", n_entries);
    return 0;
}
