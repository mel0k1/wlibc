#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <syscall.h>
#include <stdio_impl.h>

struct _IO_FILE __stdin_st = {.fd = 0, .ungch = -1, .flags = F_STATIC};
struct _IO_FILE __stdout_st = {.fd = 1, .ungch = -1, .flags = F_STATIC};
struct _IO_FILE __stderr_st = {.fd = 2, .ungch = -1, .flags = F_STATIC};

FILE *const stdin = &__stdin_st;
FILE *const stdout = &__stdout_st;
FILE *const stderr = &__stderr_st;

static unsigned char stdin_buf[BUFSIZ];
static unsigned char stdout_buf[BUFSIZ];

static FILE *open_list;
static int open_list_lock;

static void list_lock(void)
{
    if (a_cas(&open_list_lock, 0, 1) != 0) {
        while (a_swap(&open_list_lock, 2))
            __futex_wait(&open_list_lock, 2);
    }
}

static void list_unlock(void)
{
    if (a_swap(&open_list_lock, 0) == 2)
        __futex_wake(&open_list_lock, 1);
}

void __stdio_register(FILE *f)
{
    list_lock();
    f->next = open_list;
    open_list = f;
    list_unlock();
}

void __stdio_unregister(FILE *f)
{
    list_lock();
    FILE **p = &open_list;
    while (*p && *p != f)
        p = &(*p)->next;
    if (*p)
        *p = f->next;
    list_unlock();
}

void __flock(FILE *f)
{
    if (a_cas(&f->lock, 0, 1) == 0)
        return;
    while (a_swap(&f->lock, 2))
        __futex_wait(&f->lock, 2);
}

void __funlock(FILE *f)
{
    if (a_swap(&f->lock, 0) == 2)
        __futex_wake(&f->lock, 1);
}

// F_GETFL кэшируется один раз на поток, чтобы не делать syscall на каждую операцию
void __facc_cache(FILE *f)
{
    long fl = __syscall3(SYS_fcntl, f->fd, F_GETFL, 0);
    if (fl < 0)
        return;
    int acc = (int)fl & O_ACCMODE;
    if (acc != O_WRONLY)
        f->flags |= F_CANRD;
    if (acc != O_RDONLY)
        f->flags |= F_CANWR;
}

int __faccess(FILE *f, int wr)
{
    if (wr && !(f->flags & F_CANWR))
        return 0;
    if (!wr && !(f->flags & F_CANRD))
        return 0;
    return 1;
}

void __stdio_init(void)
{
    __facc_cache(&__stdin_st);
    __facc_cache(&__stdout_st);
    __facc_cache(&__stderr_st);

    __stdin_st.buf = stdin_buf;
    __stdin_st.bufcap = sizeof(stdin_buf);
    __stdin_st.bufmode = _IOFBF;
    __stdout_st.buf = stdout_buf;
    __stdout_st.bufcap = sizeof(stdout_buf);
    // строчная буферизация stdout только на терминале
    unsigned char tbuf[32];
    __stdout_st.bufmode =
        __syscall3(SYS_ioctl, 1, 0x5401 /*TCGETS*/, (long)tbuf) == 0 ? _IOLBF
                                                                     : _IOFBF;
    __stderr_st.bufmode = _IONBF;
}

static int write_all(int fd, const unsigned char *p, size_t n)
{
    while (n) {
        ssize_t r = write(fd, p, n);
        if (r < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        p += r;
        n -= (size_t)r;
    }
    return 0;
}

int __flushbuf(FILE *f)
{
    size_t n = f->wpos;
    f->wpos = 0;
    if (!n)
        return 0;
    if (write_all(f->fd, f->buf, n) < 0) {
        f->flags |= F_ERR;
        return EOF;
    }
    return 0;
}

void __wbuf_put(FILE *f, const unsigned char *p, size_t n)
{
    if (f->flags & F_ERR)
        return;
    if (f->bufmode == _IONBF || !f->buf ||
        (f->wpos == 0 && n >= f->bufcap)) {
        if (write_all(f->fd, p, n) < 0)
            f->flags |= F_ERR;
        return;
    }
    while (n) {
        size_t k = f->bufcap - f->wpos;
        if (k > n)
            k = n;
        memcpy(f->buf + f->wpos, p, k);
        f->wpos += k;
        p += k;
        n -= k;
        if (f->wpos == f->bufcap ||
            (f->bufmode == _IOLBF && memchr(f->buf + f->wpos - k, '\n', k)))
            __flushbuf(f);
    }
}

int __refill(FILE *f)
{
    ssize_t r = read(f->fd, f->buf, f->bufcap);
    if (r < 0) {
        f->flags |= F_ERR;
        return EOF;
    }
    if (r == 0) {
        f->flags |= F_EOF;
        return EOF;
    }
    f->rend = (size_t)r;
    f->rpos = 0;
    return 0;
}

void __stdio_exit(void)
{
    for (FILE *f = open_list; f; f = f->next) {
        __flock(f);
        if ((f->flags & F_WRITE) && f->wpos)
            __flushbuf(f);
        __funlock(f);
    }
    __flock(&__stdout_st);
    if (__stdout_st.wpos)
        __flushbuf(&__stdout_st);
    __funlock(&__stdout_st);
}

int fflush(FILE *f)
{
    if (!f) {
        for (FILE *g = open_list; g; g = g->next) {
            __flock(g);
            if (g->wpos)
                __flushbuf(g);
            __funlock(g);
        }
        return 0;
    }
    __flock(f);
    int r = f->wpos ? __flushbuf(f) : 0;
    __funlock(f);
    return r;
}

int feof(FILE *f)
{
    __flock(f);
    int r = !!(f->flags & F_EOF);
    __funlock(f);
    return r;
}

int ferror(FILE *f)
{
    __flock(f);
    int r = !!(f->flags & F_ERR);
    __funlock(f);
    return r;
}

void clearerr(FILE *f)
{
    __flock(f);
    f->flags &= ~(F_EOF | F_ERR);
    __funlock(f);
}

int fileno(FILE *f)
{
    return f->fd;
}

// в ребёнке после fork других потоков нет — снимаем зависшие локи
void __stdio_fork_child(void)
{
    open_list_lock = 0;
    for (FILE *f = open_list; f; f = f->next)
        f->lock = 0;
    __stdin_st.lock = 0;
    __stdout_st.lock = 0;
    __stderr_st.lock = 0;
}
