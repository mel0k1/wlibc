#pragma once

#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define RAND_MAX 0x7fffffff

void *malloc(size_t n);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t n);
void free(void *ptr);

void exit(int code) __attribute__((noreturn));
int atexit(void (*func)(void));
void abort(void) __attribute__((noreturn));

int atoi(const char *s);
long atol(const char *s);
long strtol(const char *s, char **end, int base);

int abs(int x);
long labs(long x);

void qsort(void *base, size_t nmemb, size_t size,
           int (*cmp)(const void *, const void *));
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*cmp)(const void *, const void *));

int rand(void);
void srand(unsigned seed);
