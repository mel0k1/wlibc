#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio_impl.h>

int fseek(FILE *f, long off, int whence)
{
    __flock(f);
    f->ungch = -1;
    if ((f->flags & F_WRITE) && f->wpos && __flushbuf(f) < 0) {
        __funlock(f);
        return -1;
    }
    long adjust = 0;
    if ((f->flags & F_READ) && f->rend > f->rpos)
        adjust = (long)(f->rend - f->rpos);
    if (whence == SEEK_CUR)
        off -= adjust;
    off_t r = lseek(f->fd, off, whence);
    if (r < 0) {
        if (errno == ESPIPE)
            f->flags |= F_ERR;
        __funlock(f);
        return -1;
    }
    f->rend = 0;
    f->rpos = 0;
    f->flags &= ~(F_EOF | F_READ);
    __funlock(f);
    return 0;
}

long ftell(FILE *f)
{
    __flock(f);
    if (f->wpos && __flushbuf(f) < 0) {
        __funlock(f);
        return -1;
    }
    off_t cur = lseek(f->fd, 0, SEEK_CUR);
    if (cur < 0) {
        __funlock(f);
        return -1;
    }
    if ((f->flags & F_READ) && f->rend > f->rpos) {
        cur -= (off_t)(f->rend - f->rpos);
        if (f->ungch >= 0)
            cur--;
    }
    __funlock(f);
    return (long)cur;
}

void rewind(FILE *f)
{
    fseek(f, 0, SEEK_SET);
}
