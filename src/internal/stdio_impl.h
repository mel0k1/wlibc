#pragma once

#include <stddef.h>
#include <stdio.h>
#include <sync.h>

#define F_EOF 1
#define F_ERR 2
#define F_READ 4  // последняя операция — чтение
#define F_WRITE 8 // последняя операция — запись
#define F_STATIC 16
#define F_USERBUF 32
#define F_CANRD 64
#define F_CANWR 128

struct _IO_FILE {
    int fd;
    int flags;
    int bufmode;
    unsigned char *buf;
    size_t bufcap;
    size_t wpos;  // заполнение буфера записи
    size_t rend;  // конец валидных данных буфера чтения
    size_t rpos;  // позиция чтения в буфере
    int ungch;    // ungetc: -1 если пусто
    int lock;
    struct _IO_FILE *next;
};

extern struct _IO_FILE __stdin_st, __stdout_st, __stderr_st;

void __flock(FILE *f);
void __funlock(FILE *f);
int __flushbuf(FILE *f);
int __refill(FILE *f);
void __wbuf_put(FILE *f, const unsigned char *p, size_t n);
void __stdio_register(FILE *f);
void __stdio_unregister(FILE *f);
void __stdio_fork_child(void);
int __faccess(FILE *f, int wr);
void __facc_cache(FILE *f);
int __to_write(FILE *f);
