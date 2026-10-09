#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int system(const char *cmd)
{
    if (!cmd)
        return access("/bin/sh", X_OK) == 0;

    sigset_t chld, old;
    sigemptyset(&chld);
    sigaddset(&chld, SIGCHLD);
    sigprocmask(SIG_BLOCK, &chld, &old);

    struct sigaction ign = { .sa_handler = SIG_IGN, .sa_flags = SA_RESTART };
    struct sigaction oldint, oldquit;
    sigaction(SIGINT, &ign, &oldint);
    sigaction(SIGQUIT, &ign, &oldquit);

    pid_t pid = fork();
    if (pid < 0) {
        int err = errno;
        sigaction(SIGINT, &oldint, 0);
        sigaction(SIGQUIT, &oldquit, 0);
        sigprocmask(SIG_SETMASK, &old, 0);
        errno = err;
        return -1;
    }

    if (pid == 0) {
        sigaction(SIGINT, &oldint, 0);
        sigaction(SIGQUIT, &oldquit, 0);
        sigprocmask(SIG_SETMASK, &old, 0);
        char *sh_argv[] = { "sh", "-c", (char *)cmd, 0 };
        execve("/bin/sh", sh_argv, environ);
        _exit(127);
    }

    int status = 0;
    pid_t r;
    do {
        r = waitpid(pid, &status, 0);
    } while (r < 0 && errno == EINTR);

    sigaction(SIGINT, &oldint, 0);
    sigaction(SIGQUIT, &oldquit, 0);
    sigprocmask(SIG_SETMASK, &old, 0);

    if (r < 0)
        return -1;
    return status;
}
