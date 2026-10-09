#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>

int execve(const char *path, char *const argv[], char *const envp[])
{
    return (int)__syscall_ret(__syscall3(SYS_execve, (long)path, (long)argv,
                                         (long)envp));
}

int execv(const char *path, char *const argv[])
{
    return execve(path, argv, environ);
}

static int execvpe(const char *file, char *const argv[]);

// varargs читаются вплоть до NULL-стража: он сам ложится в argv[argc]
#define BUILD_ARGV(first_var)                                                  \
    va_list ap;                                                                \
    va_start(ap, first_var);                                                   \
    int argc = 1;                                                              \
    va_list n;                                                                 \
    va_copy(n, ap);                                                            \
    while (va_arg(n, const char *))                                            \
        argc++;                                                                \
    va_end(n);                                                                 \
    char **argv = __builtin_alloca(sizeof(char *) * (argc + 1));               \
    argv[0] = (char *)first_var;                                               \
    for (int i = 1; i <= argc; i++)                                            \
        argv[i] = va_arg(ap, char *);

int execl(const char *path, const char *arg0, ...)
{
    BUILD_ARGV(arg0)
    va_end(ap);
    return execve(path, argv, environ);
}

int execle(const char *path, const char *arg0, ...)
{
    BUILD_ARGV(arg0)
    char **envp = va_arg(ap, char **);
    va_end(ap);
    return execve(path, argv, envp);
}

int execlp(const char *file, const char *arg0, ...)
{
    BUILD_ARGV(arg0)
    va_end(ap);
    return execvpe(file, argv);
}

static int execvpe(const char *file, char *const argv[])
{
    if (!*file) {
        errno = ENOENT;
        return -1;
    }
    if (strchr(file, '/'))
        return execve(file, argv, environ);

    const char *path = getenv("PATH");
    if (!path)
        path = "/usr/local/bin:/bin:/usr/bin";

    char buf[PATH_MAX];
    const char *p = path;
    int eaccess = 0;
    for (;;) {
        const char *sep = strchr(p, ':');
        size_t dlen = sep ? (size_t)(sep - p) : strlen(p);
        size_t flen = strlen(file);
        const char *dir = p;
        if (!dlen) { // пустой элемент PATH — текущий каталог
            dir = ".";
            dlen = 1;
        }
        if (dlen + 1 + flen + 1 <= sizeof(buf)) {
            memcpy(buf, dir, dlen);
            buf[dlen] = '/';
            memcpy(buf + dlen + 1, file, flen + 1);
            execve(buf, argv, environ);
            if (errno == EACCES)
                eaccess = 1;
            else if (errno != ENOENT && errno != ENOTDIR)
                return -1;
        }
        if (!sep)
            break;
        p = sep + 1;
    }
    errno = eaccess ? EACCES : ENOENT;
    return -1;
}

int execvp(const char *file, char *const argv[])
{
    return execvpe(file, argv);
}
