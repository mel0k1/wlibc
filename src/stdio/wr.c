#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdio_impl.h>

int __to_write(FILE *f)
{
    if (!__faccess(f, 1)) {
        f->flags |= F_ERR;
        return -1;
    }
    if (f->flags & F_READ) {
        // переключение направления: откатываем позицию на непрочитанное
        if (f->rend > f->rpos) {
            off_t back = (off_t)(f->rend - f->rpos);
            if (lseek(f->fd, -back, SEEK_CUR) >= 0) {
                f->rend = 0;
                f->rpos = 0;
            }
        } else {
            f->rend = 0;
            f->rpos = 0;
        }
        f->flags &= ~F_READ;
    }
    f->flags |= F_WRITE;
    return 0;
}

size_t fwrite(const void *src, size_t sz, size_t nm, FILE *f)
{
    if (!sz || !nm)
        return 0;
    if (nm > (size_t)-1 / sz)
        return 0;
    size_t total = sz * nm;
    __flock(f);
    if (__to_write(f) < 0 || f->flags & F_ERR) {
        __funlock(f);
        return 0;
    }
    __wbuf_put(f, src, total);
    __funlock(f);
    return f->flags & F_ERR ? 0 : nm;
}

int fputc(int c, FILE *f)
{
    unsigned char ch = (unsigned char)c;
    __flock(f);
    if (__to_write(f) < 0) {
        __funlock(f);
        return EOF;
    }
    __wbuf_put(f, &ch, 1);
    int r = f->flags & F_ERR ? EOF : ch;
    __funlock(f);
    return r;
}

int putc(int c, FILE *f)
{
    return fputc(c, f);
}

int putchar(int c)
{
    return fputc(c, stdout);
}

int fputs(const char *s, FILE *f)
{
    size_t n = strlen(s);
    __flock(f);
    if (__to_write(f) < 0) {
        __funlock(f);
        return EOF;
    }
    if (n)
        __wbuf_put(f, (const unsigned char *)s, n);
    int r = f->flags & F_ERR ? EOF : 0;
    __funlock(f);
    return r;
}

int puts(const char *s)
{
    if (fputs(s, stdout) == EOF)
        return EOF;
    return putchar('\n') == EOF ? EOF : 0;
}
