#pragma once

#include <stddef.h>

int __fd_write_all(int fd, const char *buf, size_t n);

void __stdio_init(void);
void __stdio_exit(void);
void __stdio_fork_child(void);
void __malloc_fork_child(void);

__attribute__((noreturn)) void __libc_start_main(long argc, char **argv);
