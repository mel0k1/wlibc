#pragma once

typedef __builtin_va_list va_list;

#define va_start(v, last) __builtin_va_start(v, last)
#define va_end(v)         __builtin_va_end(v)
#define va_arg(v, type)   __builtin_va_arg(v, type)
#define va_copy(dst, src) __builtin_va_copy(dst, src)
