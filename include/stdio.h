#pragma once

#include <stddef.h>
#include <stdarg.h>

#define EOF (-1)

int printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int vprintf(const char *fmt, va_list ap);
int dprintf(int fd, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int vdprintf(int fd, const char *fmt, va_list ap);
int sprintf(char *buf, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int snprintf(char *buf, size_t n, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));
int vsprintf(char *buf, const char *fmt, va_list ap);
int vsnprintf(char *buf, size_t n, const char *fmt, va_list ap);

int puts(const char *s);
int putchar(int c);
int getchar(void);
