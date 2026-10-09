#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdio_impl.h>

static void to_read(FILE *f)
{
    if (f->flags & F_WRITE) {
        if (f->wpos)
            __flushbuf(f);
        f->flags &= ~F_WRITE;
    }
    f->flags |= F_READ;
}

static size_t read_common(FILE *f, unsigned char *d, size_t total)
{
    size_t done = 0;
    __flock(f);
    if (!__faccess(f, 0)) {
        f->flags |= F_ERR;
        __funlock(f);
        return 0;
    }
    to_read(f);
    if (f->ungch >= 0) {
        *d++ = (unsigned char)f->ungch;
        f->ungch = -1;
        done++;
    }
    while (done < total && !(f->flags & (F_EOF | F_ERR))) {
        if (f->rpos < f->rend) {
            size_t k = f->rend - f->rpos;
            if (k > total - done)
                k = total - done;
            memcpy(d, f->buf + f->rpos, k);
            f->rpos += k;
            d += k;
            done += k;
            continue;
        }
        // без буфера или крупный запрос — читаем напрямую
        if (f->bufmode == _IONBF || !f->buf || total - done >= f->bufcap) {
            ssize_t r = read(f->fd, d, total - done);
            if (r < 0) {
                if (errno == EINTR)
                    continue;
                f->flags |= F_ERR;
                break;
            }
            if (r == 0) {
                f->flags |= F_EOF;
                break;
            }
            d += r;
            done += (size_t)r;
        } else if (__refill(f) < 0) {
            break;
        }
    }
    __funlock(f);
    return done;
}

size_t fread(void *dst, size_t sz, size_t nm, FILE *f)
{
    if (!sz || !nm)
        return 0;
    if (nm > (size_t)-1 / sz)
        return 0;
    return read_common(f, dst, sz * nm) / sz;
}

int fgetc(FILE *f)
{
    unsigned char c;
    if (read_common(f, &c, 1) != 1)
        return EOF;
    return c;
}

int getc(FILE *f)
{
    return fgetc(f);
}

int getchar(void)
{
    return fgetc(stdin);
}

int ungetc(int c, FILE *f)
{
    if (c == EOF)
        return EOF;
    __flock(f);
    if (f->ungch >= 0) {
        __funlock(f);
        return EOF;
    }
    f->flags &= ~F_EOF;
    f->ungch = (unsigned char)c;
    __funlock(f);
    return c;
}

char *fgets(char *dst, int n, FILE *f)
{
    if (n < 2)
        return 0;
    __flock(f);
    if (!__faccess(f, 0)) {
        f->flags |= F_ERR;
        __funlock(f);
        return 0;
    }
    to_read(f);
    __funlock(f);
    char *d = dst;
    int c;
    while (d < dst + n - 1 && (c = fgetc(f)) != EOF) {
        *d++ = (char)c;
        if (c == '\n')
            break;
    }
    if (d == dst)
        return 0;
    *d = 0;
    return dst;
}
