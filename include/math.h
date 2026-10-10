#pragma once

// FLT_EVAL_METHOD == 0 на x86_64 (SSE), вычисления идут в ширине операнда
typedef float float_t;
typedef double double_t;

#define HUGE_VAL (__builtin_huge_val())
#define HUGE_VALF (__builtin_huge_valf())
#define INFINITY (__builtin_inff())
#define NAN (__builtin_nanf(""))

#define FP_NAN 0
#define FP_INFINITE 1
#define FP_ZERO 2
#define FP_SUBNORMAL 3
#define FP_NORMAL 4

#define fpclassify(x) __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL, \
                                           FP_SUBNORMAL, FP_ZERO, x)
#define FP_ILOGB0 (-0x7fffffff - 1)
#define FP_ILOGBNAN 0x7fffffff

#define isfinite(x) __builtin_isfinite(x)
#define isnan(x) __builtin_isnan(x)
#define isinf(x) __builtin_isinf(x)
#define signbit(x) __builtin_signbit(x)

#define M_E 2.7182818284590452354
#define M_LOG2E 1.4426950408889634074
#define M_LOG10E 0.43429448190325182765
#define M_LN2 0.69314718055994530942
#define M_LN10 2.30258509299404568402
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.78539816339744830962
#define M_1_PI 0.31830988618379067154
#define M_2_PI 0.63661977236758134308
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2 1.41421356237309504880
#define M_SQRT1_2 0.70710678118654752440

double acos(double);
double asin(double);
double atan(double);
double atan2(double, double);
double cos(double);
double sin(double);
double tan(double);
double acosh(double);
double asinh(double);
double atanh(double);
double cosh(double);
double sinh(double);
double tanh(double);
double exp(double);
double exp2(double);
double expm1(double);
double frexp(double, int *);
int ilogb(double);
double ldexp(double, int);
double log(double);
double log10(double);
double log1p(double);
double log2(double);
double logb(double);
double modf(double, double *);
double scalbn(double, int);
double cbrt(double);
double fabs(double);
double hypot(double, double);
double pow(double, double);
double sqrt(double);
double ceil(double);
double floor(double);
double nearbyint(double);
double rint(double);
double round(double);
double trunc(double);
double fmod(double, double);
double copysign(double, double);
double nan(const char *);
double fdim(double, double);
double fmax(double, double);
double fmin(double, double);

float acosf(float);
float asinf(float);
float atanf(float);
float atan2f(float, float);
float cosf(float);
float sinf(float);
float tanf(float);
float acoshf(float);
float asinhf(float);
float atanhf(float);
float coshf(float);
float sinhf(float);
float tanhf(float);
float expf(float);
float exp2f(float);
float expm1f(float);
float frexpf(float, int *);
int ilogbf(float);
float ldexpf(float, int);
float logf(float);
float log10f(float);
float log1pf(float);
float log2f(float);
float logbf(float);
float modff(float, float *);
float scalbnf(float, int);
float cbrtf(float);
float fabsf(float);
float hypotf(float, float);
float powf(float, float);
float sqrtf(float);
float ceilf(float);
float floorf(float);
float nearbyintf(float);
float rintf(float);
float roundf(float);
float truncf(float);
float fmodf(float, float);
float copysignf(float, float);
float nanf(const char *);
float fdimf(float, float);
float fmaxf(float, float);
float fminf(float, float);
