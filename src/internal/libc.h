#pragma once

int __fd_write_all(int fd, const char *buf, size_t n);

__attribute__((noreturn)) void __libc_start_main(long argc, char **argv);
