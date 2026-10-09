#pragma once

typedef long (*vdso_clock_gettime_fn)(long, void *);
typedef long (*vdso_clock_getres_fn)(long, void *);
typedef long (*vdso_gettimeofday_fn)(void *, void *);
typedef long (*vdso_time_fn)(long *);

extern vdso_clock_gettime_fn __vdso_clock_gettime_f;
extern vdso_clock_getres_fn __vdso_clock_getres_f;
extern vdso_gettimeofday_fn __vdso_gettimeofday_f;
extern vdso_time_fn __vdso_time_f;
extern int __vdso_active;

void __init_vdso(void *ehdr);
