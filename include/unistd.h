#pragma once

#include <stddef.h>
#include <sys/types.h>

extern char **environ;

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

ssize_t read(int fd, void *buf, size_t n);
ssize_t write(int fd, const void *buf, size_t n);
int close(int fd);
off_t lseek(int fd, off_t off, int whence);
int unlink(const char *path);

pid_t getpid(void);
uid_t getuid(void);
gid_t getgid(void);

void _exit(int code) __attribute__((noreturn));
unsigned sleep(unsigned seconds);
