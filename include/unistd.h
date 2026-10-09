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

#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4

ssize_t read(int fd, void *buf, size_t n);
ssize_t write(int fd, const void *buf, size_t n);
int close(int fd);
off_t lseek(int fd, off_t off, int whence);
int unlink(const char *path);
int pipe(int fds[2]);
int dup2(int oldfd, int newfd);
int access(const char *path, int mode);

pid_t getpid(void);
pid_t getppid(void);
uid_t getuid(void);
gid_t getgid(void);

void _exit(int code) __attribute__((noreturn));
unsigned sleep(unsigned seconds);

pid_t fork(void);
int execve(const char *path, char *const argv[], char *const envp[]);
int execv(const char *path, char *const argv[]);
int execle(const char *path, const char *arg0, ...);
int execl(const char *path, const char *arg0, ...);
int execvp(const char *file, char *const argv[]);
int execlp(const char *file, const char *arg0, ...);
