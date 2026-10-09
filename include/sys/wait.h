#pragma once

#include <sys/types.h>

#define WNOHANG 1
#define WUNTRACED 2
#define WCONTINUED 8

#define WEXITSTATUS(s) (((s)&0xff00) >> 8)
#define WTERMSIG(s) ((s)&0x7f)
#define WSTOPSIG(s) WEXITSTATUS(s)
#define WCOREDUMP(s) ((s)&0x80)
#define WIFEXITED(s) (!WTERMSIG(s))
#define WIFSIGNALED(s) (((s)&0xffff) - 1U < 0xffU)
#define WIFSTOPPED(s) ((short)(((s)&0xffff) * 0x10001U) > 0x7f00)
#define WIFCONTINUED(s) ((s) == 0xffff)

pid_t wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);
