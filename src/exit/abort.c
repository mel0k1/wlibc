#include <signal.h>
#include <stdlib.h>
#include <syscall.h>

void abort(void)
{
    sigset_t unblock;
    sigemptyset(&unblock);
    sigaddset(&unblock, SIGABRT);
    sigprocmask(SIG_UNBLOCK, &unblock, 0);

    raise(SIGABRT);

    // обработчик вернулся: снимаем его и повторяем
    signal(SIGABRT, SIG_DFL);
    sigprocmask(SIG_UNBLOCK, &unblock, 0);
    raise(SIGABRT);

    // и это не сработало (SIGABRT заблокирован): последний выход
    __syscall1(SYS_exit_group, 127);
    for (;;)
        __syscall1(SYS_exit, 127);
}
