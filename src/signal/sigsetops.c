#include <errno.h>
#include <signal.h>

int sigemptyset(sigset_t *set)
{
    set->__bits[0] = 0;
    return 0;
}

int sigfillset(sigset_t *set)
{
    set->__bits[0] = ~0UL;
    return 0;
}

int sigaddset(sigset_t *set, int sig)
{
    if (sig <= 0 || sig >= _NSIG) {
        errno = EINVAL;
        return -1;
    }
    set->__bits[0] |= 1UL << (sig - 1);
    return 0;
}

int sigdelset(sigset_t *set, int sig)
{
    if (sig <= 0 || sig >= _NSIG) {
        errno = EINVAL;
        return -1;
    }
    set->__bits[0] &= ~(1UL << (sig - 1));
    return 0;
}

int sigismember(const sigset_t *set, int sig)
{
    if (sig <= 0 || sig >= _NSIG) {
        errno = EINVAL;
        return -1;
    }
    return (set->__bits[0] >> (sig - 1)) & 1;
}
