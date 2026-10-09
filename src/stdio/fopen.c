#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio_impl.h>

static int mode_flags(const char *mode)
{
    int f;
    if (mode[0] == 'r')
        f = O_RDONLY;
    else if (mode[0] == 'w')
        f = O_WRONLY | O_CREAT | O_TRUNC;
    else if (mode[0] == 'a')
        f = O_WRONLY | O_CREAT | O_APPEND;
    else
        return -1;
    if (strchr(mode, '+'))
        f = (f & ~O_ACCMODE) | O_RDWR;
    return f;
}

static void f_setup(FILE *f, int fd)
{
    f->fd = fd;
    f->ungch = -1;
    f->bufmode = _IOFBF;
    __facc_cache(f);
    f->buf = malloc(BUFSIZ);
    if (f->buf)
        f->bufcap = BUFSIZ;
    __stdio_register(f);
}

FILE *fopen(const char *path, const char *mode)
{
    int fl = mode_flags(mode);
    if (fl < 0) {
        errno = EINVAL;
        return 0;
    }
    int fd = open(path, fl, 0666);
    if (fd < 0)
        return 0;
    FILE *f = calloc(1, sizeof(*f));
    if (!f) {
        close(fd);
        return 0;
    }
    f_setup(f, fd);
    return f;
}

FILE *fdopen(int fd, const char *mode)
{
    if (mode_flags(mode) < 0) {
        errno = EINVAL;
        return 0;
    }
    FILE *f = calloc(1, sizeof(*f));
    if (!f)
        return 0;
    f_setup(f, fd);
    return f;
}

int fclose(FILE *f)
{
    if (!f)
        return EOF;
    __flock(f);
    __stdio_unregister(f);
    int r = 0;
    if (f->wpos && __flushbuf(f) < 0)
        r = EOF;
    if (close(f->fd) < 0)
        r = EOF;
    if (!(f->flags & F_USERBUF))
        free(f->buf);
    int st = f->flags & F_STATIC;
    __funlock(f);
    if (!st)
        free(f);
    return r;
}

int setvbuf(FILE *f, char *buf, int mode, size_t size)
{
    if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF) {
        errno = EINVAL;
        return -1;
    }
    __flock(f);
    if (f->wpos)
        __flushbuf(f);
    f->bufmode = mode;
    if (mode != _IONBF && buf && size) {
        if (!(f->flags & F_USERBUF))
            free(f->buf);
        f->buf = (unsigned char *)buf;
        f->bufcap = size;
        f->flags |= F_USERBUF;
    }
    __funlock(f);
    return 0;
}

void setbuf(FILE *f, char *buf)
{
    setvbuf(f, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
}
