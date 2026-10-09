#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t got;

static void handler(int sig) { got = sig; }

int main(void)
{
    // signal(): установка и возврат старого обработчика
    __sighandler_t prev = signal(SIGUSR1, handler);
    assert(prev == SIG_DFL);
    raise(SIGUSR1);
    assert(got == SIGUSR1);

    // kill() по самому себе
    got = 0;
    assert(kill(getpid(), SIGUSR1) == 0);
    assert(got == SIGUSR1);

    // SIG_IGN
    assert(signal(SIGUSR2, SIG_IGN) == SIG_DFL);
    raise(SIGUSR2);

    // блокировка: pending до снятия маски
    sigset_t block, old, pend;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    assert(sigprocmask(SIG_BLOCK, &block, &old) == 0);
    got = 0;
    raise(SIGUSR1);
    assert(got == 0);
    assert(sigpending(&pend) == 0);
    assert(sigismember(&pend, SIGUSR1) == 1);
    assert(sigprocmask(SIG_SETMASK, &old, 0) == 0);
    assert(got == SIGUSR1);

    // sigsuspend: доставляет pending-сигнал, возвращает EINTR
    assert(sigprocmask(SIG_BLOCK, &block, 0) == 0);
    got = 0;
    raise(SIGUSR1);
    sigset_t empty;
    sigemptyset(&empty);
    errno = 0;
    assert(sigsuspend(&empty) == -1 && errno == EINTR);
    assert(got == SIGUSR1);
    assert(sigprocmask(SIG_SETMASK, &old, 0) == 0);

    // sigaction(): чтение старого действия, ошибки на SIGKILL/SIGSTOP
    struct sigaction sa = { .sa_handler = handler, .sa_flags = SA_RESTART };
    struct sigaction oldsa;
    assert(sigaction(SIGUSR1, &sa, &oldsa) == 0);
    assert(oldsa.sa_handler == handler);
    assert(sigaction(SIGUSR1, &oldsa, 0) == 0);
    errno = 0;
    assert(sigaction(SIGKILL, &sa, 0) == -1 && errno == EINVAL);
    errno = 0;
    assert(signal(SIGSTOP, handler) == SIG_ERR && errno == EINVAL);

    // sigset-операции
    sigset_t s;
    sigfillset(&s);
    assert(sigismember(&s, SIGSEGV));
    sigdelset(&s, SIGSEGV);
    assert(sigismember(&s, SIGUSR1) && !sigismember(&s, SIGSEGV));
    sigemptyset(&s);
    assert(!sigismember(&s, SIGUSR1));

    printf("test_signal passed\n");
    return 0;
}
