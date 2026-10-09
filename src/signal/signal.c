#include <signal.h>

__sighandler_t signal(int sig, __sighandler_t handler)
{
    struct sigaction act = { .sa_handler = handler, .sa_flags = SA_RESTART };
    struct sigaction old;
    if (sigaction(sig, &act, &old) < 0)
        return SIG_ERR;
    return old.sa_handler;
}
