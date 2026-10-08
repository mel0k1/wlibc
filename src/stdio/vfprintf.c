#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libc.h>

#define SINK_TMP 512

struct sink {
    int fd;
    char *mem;
    size_t cap;
    size_t len;
    int err;
    size_t tn;
    char tmp[SINK_TMP];
};

int __fd_write_all(int fd, const char *buf, size_t n)
{
    while (n) {
        ssize_t r = write(fd, buf, n);
        if (r < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        buf += r;
        n -= (size_t)r;
    }
    return 0;
}

static void sink_flush(struct sink *s)
{
    if (s->tn && !s->err && __fd_write_all(s->fd, s->tmp, s->tn) < 0)
        s->err = 1;
    s->tn = 0;
}

static void sink_out(struct sink *s, const char *p, size_t n)
{
    if (s->fd >= 0) {
        s->len += n;
        while (n) {
            size_t k = SINK_TMP - s->tn;
            if (k > n)
                k = n;
            memcpy(s->tmp + s->tn, p, k);
            s->tn += k;
            p += k;
            n -= k;
            if (s->tn == SINK_TMP)
                sink_flush(s);
        }
        return;
    }
    size_t pos = s->len;
    s->len += n;
    if (s->cap && pos < s->cap - 1) {
        if (pos + n > s->cap - 1)
            n = s->cap - 1 - pos;
        memcpy(s->mem + pos, p, n);
    }
}

static void sink_putc(struct sink *s, char c)
{
    sink_out(s, &c, 1);
}

static void pad_out(struct sink *s, char c, int n)
{
    char buf[32];
    memset(buf, c, sizeof(buf));
    while (n > 0) {
        size_t k = n > 32 ? 32 : (size_t)n;
        sink_out(s, buf, k);
        n -= (int)k;
    }
}

static const char digits_lc[] = "0123456789abcdef";
static const char digits_uc[] = "0123456789ABCDEF";

static size_t utoa(char *dst, unsigned long long v, unsigned base, int upper)
{
    char tmp[24];
    const char *d = upper ? digits_uc : digits_lc;
    size_t i = 0, k = 0;
    do {
        tmp[i++] = d[v % base];
        v /= base;
    } while (v);
    while (i)
        dst[k++] = tmp[--i];
    dst[k] = 0;
    return k;
}

static void emit_num(struct sink *s, int left, int zero, int width,
                     const char *sign, const char *prefix, const char *body,
                     size_t blen)
{
    size_t sl = strlen(sign);
    size_t pl = strlen(prefix);
    size_t total = sl + pl + blen;
    int pad = width > 0 && (size_t)width > total ? (int)((size_t)width - total) : 0;
    if (left) {
        sink_out(s, sign, sl);
        sink_out(s, prefix, pl);
        sink_out(s, body, blen);
        pad_out(s, ' ', pad);
    } else if (zero) {
        sink_out(s, sign, sl);
        sink_out(s, prefix, pl);
        pad_out(s, '0', pad);
        sink_out(s, body, blen);
    } else {
        pad_out(s, ' ', pad);
        sink_out(s, sign, sl);
        sink_out(s, prefix, pl);
        sink_out(s, body, blen);
    }
}

static void emit_int(struct sink *s, unsigned long long uv, int neg, unsigned base,
                     int upper, int alt, int plus, int space, int left, int zero,
                     int width, int prec)
{
    if (prec > 64)
        prec = 64;
    char digs[24];
    size_t dl = utoa(digs, uv, base, upper);
    char body[96];
    size_t bl = 0;
    if (prec == 0 && uv == 0)
        dl = 0;
    if (prec > 0 && dl < (size_t)prec) {
        size_t z = (size_t)prec - dl;
        while (z--)
            body[bl++] = '0';
    }
    memcpy(body + bl, digs, dl);
    bl += dl;
    const char *sign = neg ? "-"
                           : (base == 10 ? (plus ? "+" : (space ? " " : "")) : "");
    const char *prefix = "";
    if (alt) {
        if (base == 16 && uv)
            prefix = upper ? "0X" : "0x";
        else if (base == 8 && (bl == 0 || body[0] != '0'))
            prefix = "0";
    }
    emit_num(s, left, zero && prec < 0, width, sign, prefix, body, bl);
}

static void emit_f(struct sink *s, double v, int left, int zero, int width,
                   int prec, int plus, int space)
{
    if (prec < 0)
        prec = 6;
    if (prec > 40)
        prec = 40;
    int neg = 0;
    const char *special = 0;
    if (__builtin_isnan(v))
        special = "nan";
    else if (__builtin_isinf(v))
        special = "inf";
    if (special) {
        neg = __builtin_signbit(v);
        size_t total = (size_t)(neg ? 1 : 0) + strlen(special);
        int pad = width > 0 && (size_t)width > total ? (int)((size_t)width - total)
                                                     : 0;
        if (!left)
            pad_out(s, ' ', pad);
        if (neg)
            sink_putc(s, '-');
        sink_out(s, special, strlen(special));
        if (left)
            pad_out(s, ' ', pad);
        return;
    }
    neg = __builtin_signbit(v);
    double a = neg ? -v : v;
    unsigned long long ip;
    double frac;
    if (a >= 1.0e18) {
        ip = 1000000000000000000ULL;
        frac = 0.0;
    } else {
        ip = (unsigned long long)a;
        frac = a - (double)ip;
    }
    double p10 = 1.0;
    for (int i = 0; i < prec; i++)
        p10 *= 10.0;
    unsigned long long fs = (unsigned long long)(frac * p10 + 0.5);
    if (fs >= (unsigned long long)p10) {
        ip += 1;
        fs = 0;
    }
    char ibuf[24], fbuf[48], body[96];
    size_t il = utoa(ibuf, ip, 10, 0);
    size_t bl = 0;
    memcpy(body, ibuf, il);
    bl = il;
    if (prec > 0) {
        body[bl++] = '.';
        size_t fl = utoa(fbuf, fs, 10, 0);
        for (size_t i = fl; i < (size_t)prec; i++)
            body[bl++] = '0';
        memcpy(body + bl, fbuf, fl);
        bl += fl;
    }
    const char *sign = neg ? "-" : (plus ? "+" : (space ? " " : ""));
    emit_num(s, left, zero, width, sign, "", body, bl);
}

static int core(struct sink *s, const char *fmt, va_list ap)
{
    for (const char *p = fmt; *p; p++) {
        if (*p != '%') {
            sink_putc(s, *p);
            continue;
        }
        p++;
        int left = 0, zero = 0, plus = 0, space = 0, alt = 0;
        for (;; p++) {
            if (*p == '-')
                left = 1;
            else if (*p == '0')
                zero = 1;
            else if (*p == '+')
                plus = 1;
            else if (*p == ' ')
                space = 1;
            else if (*p == '#')
                alt = 1;
            else
                break;
        }
        int width = 0;
        if (*p == '*') {
            p++;
            width = va_arg(ap, int);
            if (width < 0) {
                left = 1;
                width = -width;
            }
        } else {
            while (*p >= '0' && *p <= '9') {
                width = width * 10 + (*p - '0');
                p++;
            }
        }
        int prec = -1;
        if (*p == '.') {
            p++;
            if (*p == '*') {
                p++;
                prec = va_arg(ap, int);
                if (prec < 0)
                    prec = -1;
            } else {
                prec = 0;
                while (*p >= '0' && *p <= '9') {
                    prec = prec * 10 + (*p - '0');
                    p++;
                }
            }
        }
        int lmod = 0;
        if (*p == 'h') {
            lmod = -1;
            if (p[1] == 'h') {
                lmod = -2;
                p++;
            }
        } else if (*p == 'l') {
            lmod = 1;
            if (p[1] == 'l') {
                lmod = 2;
                p++;
            }
        } else if (*p == 'z') {
            lmod = 3;
        } else if (*p == 'j') {
            lmod = 4;
        } else if (*p == 't') {
            lmod = 5;
        }
        if (lmod)
            p++;
        char c = *p;
        if (!c)
            break;
        if (c == 'd' || c == 'i') {
            long long v;
            if (lmod == 2 || lmod == 4)
                v = va_arg(ap, long long);
            else if (lmod)
                v = va_arg(ap, long);
            else
                v = va_arg(ap, int);
            if (lmod == -1)
                v = (short)v;
            else if (lmod == -2)
                v = (signed char)v;
            unsigned long long uv = (unsigned long long)v;
            int neg = v < 0;
            if (neg)
                uv = 0ULL - uv;
            emit_int(s, uv, neg, 10, 0, alt, plus, space, left, zero, width, prec);
        } else if (c == 'u' || c == 'o' || c == 'x' || c == 'X') {
            unsigned base = c == 'o' ? 8 : (c == 'u' ? 10 : 16);
            unsigned long long uv;
            if (lmod == 2 || lmod == 4)
                uv = va_arg(ap, unsigned long long);
            else if (lmod)
                uv = va_arg(ap, unsigned long);
            else
                uv = va_arg(ap, unsigned int);
            if (lmod == -1)
                uv = (unsigned short)uv;
            else if (lmod == -2)
                uv = (unsigned char)uv;
            emit_int(s, uv, 0, base, c == 'X', alt && c != 'u', 0, 0, left, zero,
                     width, prec);
        } else if (c == 'f' || c == 'F') {
            emit_f(s, va_arg(ap, double), left, zero, width, prec, plus, space);
        } else if (c == 'c') {
            char ch = (char)va_arg(ap, int);
            int pad = width > 1 ? width - 1 : 0;
            if (!left)
                pad_out(s, ' ', pad);
            sink_putc(s, ch);
            if (left)
                pad_out(s, ' ', pad);
        } else if (c == 's') {
            const char *sv = va_arg(ap, const char *);
            if (!sv)
                sv = "(null)";
            size_t sl = prec >= 0 ? strnlen(sv, (size_t)prec) : strlen(sv);
            int pad =
                width > 0 && (size_t)width > sl ? (int)((size_t)width - sl) : 0;
            if (!left)
                pad_out(s, ' ', pad);
            sink_out(s, sv, sl);
            if (left)
                pad_out(s, ' ', pad);
        } else if (c == 'p') {
            void *pv = va_arg(ap, void *);
            if (!pv) {
                int pad = width > 3 ? width - 3 : 0;
                if (!left)
                    pad_out(s, ' ', pad);
                sink_out(s, "0x0", 3);
                if (left)
                    pad_out(s, ' ', pad);
            } else {
                char hb[24];
                size_t hl = utoa(hb, (unsigned long long)(unsigned long)pv, 16, 0);
                emit_num(s, left, zero, width, "", "0x", hb, hl);
            }
        } else if (c == 'n') {
            (void)va_arg(ap, void *);
        } else if (c == '%') {
            sink_putc(s, '%');
        } else {
            sink_putc(s, '%');
            sink_putc(s, c);
        }
    }
    sink_flush(s);
    return s->err ? -1 : (int)s->len;
}

int vdprintf(int fd, const char *fmt, va_list ap)
{
    struct sink s;
    s.fd = fd;
    s.mem = 0;
    s.cap = 0;
    s.len = 0;
    s.err = 0;
    s.tn = 0;
    return core(&s, fmt, ap);
}

int vsnprintf(char *buf, size_t n, const char *fmt, va_list ap)
{
    struct sink s;
    s.fd = -1;
    s.mem = buf;
    s.cap = n;
    s.len = 0;
    s.err = 0;
    s.tn = 0;
    int r = core(&s, fmt, ap);
    if (n)
        buf[(size_t)r < n - 1 ? (size_t)r : n - 1] = 0;
    return r;
}

int vsprintf(char *buf, const char *fmt, va_list ap)
{
    struct sink s;
    s.fd = -1;
    s.mem = buf;
    s.cap = (size_t)-1;
    s.len = 0;
    s.err = 0;
    s.tn = 0;
    int r = core(&s, fmt, ap);
    buf[r] = 0;
    return r;
}
