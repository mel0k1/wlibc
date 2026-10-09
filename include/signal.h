#pragma once

#include <sys/types.h>

typedef int sig_atomic_t;
typedef void (*__sighandler_t)(int);
typedef __sighandler_t sighandler_t;

#define SIG_ERR ((__sighandler_t)-1)
#define SIG_DFL ((__sighandler_t)0)
#define SIG_IGN ((__sighandler_t)1)

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGIOT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTKFLT 16
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGURG 23
#define SIGXCPU 24
#define SIGXFSZ 25
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGWINCH 28
#define SIGIO 29
#define SIGPOLL 29
#define SIGPWR 30
#define SIGSYS 31
#define SIGUNUSED 31

#define _NSIG 65
#define SIGRTMIN 34
#define SIGRTMAX 64

typedef struct {
    unsigned long __bits[1];
} sigset_t;

#define SIGSET_NWORDS 1

#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

#define SA_NOCLDSTOP 1
#define SA_NOCLDWAIT 2
#define SA_SIGINFO 4
#define SA_ONSTACK 8
#define SA_RESTART 0x10000000
#define SA_NODEFER 0x40000000
#define SA_RESETHAND 0x80000000

struct sigaction {
    void (*sa_handler)(int);
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};

__sighandler_t signal(int sig, __sighandler_t handler);
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact);
int sigprocmask(int how, const sigset_t *set, sigset_t *old);
int sigpending(sigset_t *set);
int sigsuspend(const sigset_t *mask);
int kill(pid_t pid, int sig);
int raise(int sig);

int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int sig);
int sigdelset(sigset_t *set, int sig);
int sigismember(const sigset_t *set, int sig);
