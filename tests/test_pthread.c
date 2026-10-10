#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NTHREADS 8
#define ITERS 5000

__thread int tls_slot = 42;
__thread char tls_buf[17] = "main-thread";

static pthread_mutex_t counter_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t cond_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static long counter;
static int queue, queue_done;
static pthread_once_t once_flag = PTHREAD_ONCE_INIT;
static int once_runs;
static volatile int detached_done;

// --- pthread_sigmask / pthread_kill ---
static volatile sig_atomic_t sig_usr1_count;
static volatile pthread_t sig_target;
static pthread_mutex_t sig_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sig_cv = PTHREAD_COND_INITIALIZER;
static int sig_ready;

static void usr1_handler(int sig)
{
    (void)sig;
    sig_usr1_count++;
    sig_target = pthread_self();
}

static void *sig_waiter(void *arg)
{
    (void)arg;
    pthread_mutex_lock(&sig_mu);
    sig_ready = 1;
    pthread_cond_signal(&sig_cv);
    pthread_mutex_unlock(&sig_mu);
    while (sig_usr1_count < 3)
        sleep(0);
    return NULL;
}

static void *adder(void *arg)
{
    (void)arg;
    for (int i = 0; i < ITERS; i++) {
        pthread_mutex_lock(&counter_mu);
        counter++;
        pthread_mutex_unlock(&counter_mu);
    }
    return NULL;
}

static void *tls_worker(void *arg)
{
    long id = (long)arg;
    assert(tls_slot == 42);
    assert(strcmp(tls_buf, "main-thread") == 0);
    tls_slot = (int)id + 100;
    strcpy(tls_buf, "worker");
    close(-1);
    assert(errno == EBADF);
    // главный поток не должен увидеть наши изменения и errno
    for (volatile long k = 0; k < 10000; k++)
        ;
    return (void *)(id * 7);
}

static void *producer(void *arg)
{
    (void)arg;
    for (int i = 1; i <= 100; i++) {
        pthread_mutex_lock(&cond_mu);
        queue = i;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&cond_mu);
    }
    return NULL;
}

static void *consumer(void *arg)
{
    (void)arg;
    int last = 0;
    pthread_mutex_lock(&cond_mu);
    while (last < 100) {
        while (queue == last)
            pthread_cond_wait(&cond, &cond_mu);
        last = queue;
        assert(last > 0);
    }
    pthread_mutex_unlock(&cond_mu);
    queue_done = 1;
    return NULL;
}

static void once_fn(void)
{
    once_runs++;
    pthread_mutex_lock(&counter_mu);
    counter += 1000;
    pthread_mutex_unlock(&counter_mu);
}

static void *once_worker(void *arg)
{
    (void)arg;
    assert(pthread_once(&once_flag, once_fn) == 0);
    return NULL;
}

static void *detached_worker(void *arg)
{
    (void)arg;
    detached_done = 1;
    return NULL;
}

static void *sleeper(void *arg)
{
    (void)arg;
    sleep(1);
    return (void *)123;
}

int main(void)
{
    pthread_t t[NTHREADS];
    void *ret;

    // TLS главного потока до/после чужих потоков
    assert(tls_slot == 42);
    assert(strcmp(tls_buf, "main-thread") == 0);
    errno = 7;

    for (long i = 0; i < NTHREADS; i++)
        assert(pthread_create(&t[i], NULL, tls_worker, (void *)i) == 0);
    for (int i = 0; i < NTHREADS; i++) {
        assert(pthread_join(t[i], &ret) == 0);
        assert((long)ret == (long)i * 7);
    }
    assert(errno == 7);
    assert(tls_slot == 42);
    assert(strcmp(tls_buf, "main-thread") == 0);

    // мьютекс: N потоков * ITERS инкрементов
    for (long i = 0; i < NTHREADS; i++)
        assert(pthread_create(&t[i], NULL, adder, NULL) == 0);
    for (int i = 0; i < NTHREADS; i++)
        assert(pthread_join(t[i], &ret) == 0);
    assert(counter == (long)NTHREADS * ITERS);

    // trylock на занятом мьютексе
    pthread_mutex_lock(&counter_mu);
    assert(pthread_mutex_trylock(&counter_mu) == EBUSY);
    pthread_mutex_unlock(&counter_mu);

    // condvar: producer/consumer
    pthread_t prod, cons;
    assert(pthread_create(&cons, NULL, consumer, NULL) == 0);
    assert(pthread_create(&prod, NULL, producer, NULL) == 0);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    assert(queue_done == 1);

    // pthread_once: 8 потоков, функция один раз
    for (long i = 0; i < NTHREADS; i++)
        assert(pthread_create(&t[i], NULL, once_worker, NULL) == 0);
    for (int i = 0; i < NTHREADS; i++)
        pthread_join(t[i], &ret);
    assert(once_runs == 1);
    assert(counter == (long)NTHREADS * ITERS + 1000);

    // detach: поток завершается и прибирает за собой сам
    pthread_t d;
    assert(pthread_create(&d, NULL, detached_worker, NULL) == 0);
    assert(pthread_detach(d) == 0);
    assert(pthread_detach(d) == EINVAL);
    while (!detached_done)
        sleep(0);
    sleep(1); // даём detach-пути выгрузить стек

    // join ждёт живого потока
    pthread_t s;
    assert(pthread_create(&s, NULL, sleeper, NULL) == 0);
    assert(pthread_join(s, &ret) == 0);
    assert((long)ret == 123);

    // self/equal
    assert(pthread_equal(pthread_self(), pthread_self()));
    assert(!pthread_equal(pthread_self(), s));

    // pthread_sigmask: заблокированный сигнал ждёт в pending
    struct sigaction sa = { .sa_handler = usr1_handler };
    sigemptyset(&sa.sa_mask);
    assert(sigaction(SIGUSR1, &sa, NULL) == 0);

    sigset_t block, oldset;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    assert(pthread_sigmask(SIG_BLOCK, &block, &oldset) == 0);
    assert(sigismember(&oldset, SIGUSR1) == 0);
    raise(SIGUSR1);
    assert(sig_usr1_count == 0);
    sigset_t pend;
    sigemptyset(&pend);
    assert(sigpending(&pend) == 0);
    assert(sigismember(&pend, SIGUSR1) == 1);
    assert(pthread_sigmask(SIG_UNBLOCK, &block, NULL) == 0);
    assert(sig_usr1_count == 1);
    sig_usr1_count = 0;

    // SIG_SETMASK возвращает прежнюю маску
    sigset_t chk;
    assert(pthread_sigmask(SIG_SETMASK, &oldset, NULL) == 0);
    assert(pthread_sigmask(SIG_SETMASK, NULL, &chk) == 0);
    assert(sigismember(&chk, SIGUSR1) == 0);

    // pthread_kill: адресная доставка SIGUSR1 в живой поток
    pthread_t wt;
    assert(pthread_create(&wt, NULL, sig_waiter, NULL) == 0);
    pthread_mutex_lock(&sig_mu);
    while (!sig_ready)
        pthread_cond_wait(&sig_cv, &sig_mu);
    pthread_mutex_unlock(&sig_mu);

    assert(pthread_kill(wt, 0) == 0);
    assert(pthread_kill(wt, _NSIG) == EINVAL);
    assert(pthread_kill(wt, -3) == EINVAL);
    // обычные сигналы не ставятся в очередь: следующий kill шлём только
    // после доставки предыдущего, иначе доставки коалесцируют
    for (int i = 0; i < 3; i++) {
        int before = sig_usr1_count;
        assert(pthread_kill(wt, SIGUSR1) == 0);
        while (sig_usr1_count == before)
            sleep(0);
    }
    pthread_join(wt, NULL);
    assert(sig_usr1_count == 3);
    assert(pthread_equal(sig_target, wt));

    printf("test_pthread passed\n");
    return 0;
}
