#include <errno.h>
#include <signal.h>
#include <ksigaction.h>
#include <syscall.h>

extern void __restore_rt(void);

int sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
    if (sig <= 0 || sig >= _NSIG || sig == SIGKILL || sig == SIGSTOP) {
        errno = EINVAL;
        return -1;
    }

    struct k_sigaction ka, ko;
    if (act) {
        ka.handler = act->sa_handler;
        ka.flags = (unsigned long)act->sa_flags | K_SA_RESTORER;
        ka.restorer = __restore_rt;
        ka.mask = act->sa_mask.__bits[0];
    }

    long r = __syscall4(SYS_rt_sigaction, sig, act ? (long)&ka : 0,
                        oact ? (long)&ko : 0, 8);
    if (__syscall_ret(r) == -1)
        return -1;

    if (oact) {
        oact->sa_handler = ko.handler;
        oact->sa_mask.__bits[0] = ko.mask;
        oact->sa_flags = (int)ko.flags & ~K_SA_RESTORER;
        oact->sa_restorer = 0;
    }
    return 0;
}
