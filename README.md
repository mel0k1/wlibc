# wlibc

> Portable C standard library for Linux on x86_64, written from scratch.
> Takes design cues from [m[musl](https://musl.libc.org), [mlibc](https://github.com/managarm/mlibc) and [glibc](https://www.gnu.org/software/libc/).

`wlibc` is a small, freestanding-first libc: it does not depend on the host
libc at all (the whole library builds with `-nostdinc`), ships its own
crt0 and links everything statically. The goal is a codebase you can actually
read end-to-end and port to a new OS or architecture by adding a single
`sysdeps` directory.

## Status

| Area | State |
|------|-------|
| crt0 (`_start`) + `__libc_start_main` | done |
| Linux/x86_64 sysdep layer (inline-asm syscalls, errno mapping) | done |
| `string.h`: `mem*`, `str*`, `strdup`, `strnlen` | done |
| `stdio.h`: `printf` family — `d/i/u/o/x/X/c/s/p/f`, flags, width, precision, length modifiers; `puts`, `putchar`, `getchar` | done |
| `stdlib.h`: `malloc`/`calloc`/`realloc`/`free` (mmap-backed), `strtol`/`atoi`/`atol`, `qsort`/`bsearch`, `rand`/`srand` | done |
| `unistd.h` / `fcntl.h` / `sys/mman.h`: `read`, `write`, `open`, `close`, `lseek`, `unlink`, `mmap`, `mprotect`, `munmap`, ... | done |
| `errno`, `strerror`, `assert`, `ctype` | done |
| TLS: `__thread` via local-exec model, per-thread `errno` / `strerror` buf / `rand` state | done |
| pthreads: `create`/`join`/`detach`/`exit` on `clone(2)`, futex mutexes / condvars / `pthread_once` | done |
| buffered `FILE*` stdio: `fopen`/`fdopen`/`fclose`, `fread`/`fwrite`, `fgetc`/`fputc`, `fgets`/`fputs`, `fseek`/`ftell`/`rewind`, `ungetc`, `setvbuf`, `fflush` | done |
| signals: `sigaction`/`signal`, `sigprocmask`/`sigpending`/`sigsuspend`, sigset ops, `kill`/`raise`, sigreturn trampoline | done |
| process: `fork` + `pthread_atfork`, `execve`/`execl*`/`execv*` (PATH search), `waitpid`/`wait`, `system`, `getenv` | done |
| time: `clock_gettime`/`clock_getres`/`gettimeofday`/`time`/`clock`/`nanosleep` with vDSO fast path, syscall fallback | done |
| math: double/float from fdlibm — `exp`/`log`/`pow`/`sin`... (40 double + 42 float functions), `float.h` | done |
| calendar: `gmtime`/`gmtime_r`/`localtime(_r)`, `mktime`/`timegm`, `asctime`/`ctime`, `strftime`, `difftime` | done |
| `pthread_sigmask`, `pthread_kill` | done |
| dynamic linking | not planned for now |
| more architectures (aarch64, riscv64) | planned |

Known simplifications: `printf` has no `%n` (silently ignored) and `%f` is a
fixed-point approximation without exponent form; pthreads has no cancellation,
rwlocks, timed waits or per-thread TS yet; condvars use a sequence-counter
protocol (spurious wakeups are allowed, which `while (!pred) wait` loops
tolerate by design); signals have no `SA_SIGINFO`/`siginfo_t`, no `sigaltstack`
and no `sigaltstack` yet; `fork` in a multithreaded process only resets
stdio and heap locks — memory mapped by other threads' stacks stays mapped
in the child; calendar functions treat local time as UTC (`TZ` is not
parsed yet) and `strftime` has no ISO week specifiers (`%G`/`%g`/`%V`);
math is ported from fdlibm (via musl) and follows the musl conventions:
transcendentals do not set `errno`, there is no `fenv.h` control
(`nearbyint` behaves like `rint`), and `fma`/`erf`/`lgamma`/Bessel are
still on the roadmap; no timers or `posix_spawn` yet.

## Design

```
include/          public headers (self-contained, no host libc)
src/              arch-independent core: never issues a raw syscall
  internal/       internal headers shared by the core
  string/ stdio/ stdlib/ malloc/ unistd/ fcntl/ mman/ signal/ time/ errno/ exit/ ctype/ assert/
sysdeps/
  linux/x86_64/   syscall numbers, inline-asm entry points, crt0, sigreturn
```

The core talks to the outside world only through the sysdeps interface, so
porting to another OS or architecture means adding one directory under
`sysdeps/` — the same idea mlibc uses. The allocator reserves a large virtual
region with `PROT_NONE` and commits pages on demand via `mprotect`, which
keeps the heap contiguous and makes block coalescing trivial.

Signals use the kernel `rt_sigaction` layout with a mandatory `SA_RESTORER`
trampoline (`__restore_rt` in `sysdeps/linux/x86_64/sigreturn.s`), since the
x86_64 kernel provides no default restorer. On `fork()` the child reclaims
stdio and heap locks and re-arms `set_tid_address`, so a single-threaded
copy of a multithreaded parent stays usable.

Math is ported from fdlibm (the musl 1.1.15 lineage of the Sun sources,
copyright notices preserved): bit-exact scalar algorithms with union-based
word access, no floating-point environment dependencies. All math lives in
`libc.a` — there is no separate `libm`.

`clock_gettime` and friends first probe the vDSO (`AT_SYSINFO_EHDR` is parsed
in `__libc_start_main`, symbols resolved via DT_GNU_HASH/DT_HASH with a load
bias, mirroring musl) and fall back to the raw syscall when the vDSO declines
an unknown clock id.

Static linking only, by design. No dynamic loader yet.

## Building

```
make            # builds libc.a and obj/crt1.o
make test       # builds and runs the test suite
```

Requires gcc or clang on Linux/x86_64. Nothing else.

## Using wlibc

```
gcc -nostdinc -ffreestanding -I/path/to/wlibc/include -c prog.c -o prog.o
gcc -static -nostdlib -o prog prog.o /path/to/wlibc/obj/crt1.o /path/to/wlibc/libc.a
```

or with the helper script (after `make`):

```
./scripts/wlibc-gcc -o prog prog.c
```

### hello world

```c
#include <stdio.h>

int main(void)
{
    printf("hello from wlibc\n");
    return 0;
}
```

## Roadmap

- math completion: `fma`, `erf`/`erfc`, `lgamma`/`tgamma`, Bessel, long double
- pthreads completion: timed waits, rwlocks, cancellation
- `SA_SIGINFO`, `sigaltstack`, `posix_spawn`, `poll`/`select`, `opendir`/`readdir`
- `TZ` parsing for `localtime`/`mktime`, ISO week specifiers in `strftime`
- aarch64 and riscv64 sysdeps
- locale skeleton (C + UTF-8)

## Acknowledgements

- [musl](https://git.musl-libc.org/cgit/musl/) — the cleanest libc source to learn from
- [mlibc](https://github.com/managarm/mlibc) — the sysdeps architecture
- [glibc](https://sourceware.org/glibc/) — the reference behavior

## Contributing

Issues and PRs are welcome. Keep the core syscall-free, keep new OS/arch
support inside `sysdeps/`, and add a test for anything user-visible.

## License

MIT — see [LICENSE](LICENSE).
