#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define PATH "/tmp/wlibc_fork_exec_test.dat"

static int atfork_prepare, atfork_parent, atfork_child;

static void af_prepare(void) { atfork_prepare++; }
static void af_parent(void) { atfork_parent++; }
static void af_child(void) { atfork_child++; }

// держит heap_lock в момент fork — ребёнок должен не зависнуть на malloc
static void *hammer(void *arg)
{
    (void)arg;
    for (int i = 0; i < 20000; i++) {
        void *p = malloc(64);
        assert(p);
        free(p);
    }
    return NULL;
}

int main(void)
{
    assert(getenv("PATH") != NULL);
    assert(pthread_atfork(af_prepare, af_parent, af_child) == 0);

    // fork + _exit: код выхода через waitpid
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0)
        _exit(42);
    int st = -1;
    assert(waitpid(pid, &st, 0) == pid);
    assert(WIFEXITED(st) && WEXITSTATUS(st) == 42);

    // fork из многопоточного процесса: ребёнок аллоцирует и пишет файл
    pthread_t th;
    assert(pthread_create(&th, NULL, hammer, NULL) == 0);
    pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        char *p = malloc(999);
        assert(p);
        memset(p, 'x', 999);
        FILE *f = fopen(PATH, "w");
        assert(f);
        fwrite(p, 1, 999, f);
        fclose(f);
        free(p);
        _exit(0);
    }
    assert(pthread_join(th, NULL) == 0);
    assert(waitpid(pid, &st, 0) == pid);
    assert(WIFEXITED(st) && WEXITSTATUS(st) == 0);
    FILE *f = fopen(PATH, "r");
    assert(f);
    char buf[1100];
    memset(buf, 0, sizeof(buf));
    assert(fread(buf, 1, 999, f) == 999);
    fclose(f);
    for (int i = 0; i < 999; i++)
        assert(buf[i] == 'x');

    // execvp: поиск в PATH, stdout перенаправлен в файл
    pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        int fd = open(PATH, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        assert(fd >= 0);
        assert(dup2(fd, 1) == 1);
        char *argv[] = { "echo", "forkexec-ok", NULL };
        execvp("echo", argv);
        _exit(127);
    }
    assert(waitpid(pid, &st, 0) == pid);
    assert(WIFEXITED(st) && WEXITSTATUS(st) == 0);
    f = fopen(PATH, "r");
    assert(f);
    assert(fgets(buf, sizeof(buf), f) != NULL);
    fclose(f);
    assert(strcmp(buf, "forkexec-ok\n") == 0);

    // execle: своё окружение
    pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        int fd = open(PATH, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        assert(fd >= 0);
        assert(dup2(fd, 1) == 1);
        char *envp[] = { "WLIBC_FOO=env-ok", NULL };
        execle("/bin/sh", "sh", "-c", "printf %s $WLIBC_FOO", (char *)NULL,
               envp);
        _exit(127);
    }
    assert(waitpid(pid, &st, 0) == pid);
    assert(WIFEXITED(st) && WEXITSTATUS(st) == 0);
    f = fopen(PATH, "r");
    assert(f);
    assert(fgets(buf, sizeof(buf), f) != NULL);
    fclose(f);
    assert(strcmp(buf, "env-ok") == 0);

    // system
    st = system("exit 7");
    assert(WIFEXITED(st) && WEXITSTATUS(st) == 7);
    assert(system(NULL) != 0);

    // waitpid WNOHANG до завершения ребёнка
    pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        for (volatile long i = 0; i < 10000000; i++)
            ;
        _exit(5);
    }
    st = 0;
    assert(waitpid(pid, &st, WNOHANG) == 0);
    assert(waitpid(pid, &st, 0) == pid);
    assert(WEXITSTATUS(st) == 5);

    // atfork-хуки: в родителе prepare/parent на каждом форке, child — в ребёнке
    assert(atfork_prepare == 6 && atfork_parent == 6 && atfork_child == 0);
    pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        if (atfork_prepare != 7 || atfork_parent != 6 || atfork_child != 1)
            _exit(1);
        _exit(0);
    }
    assert(waitpid(pid, &st, 0) == pid);
    assert(WIFEXITED(st) && WEXITSTATUS(st) == 0);

    // getppid
    pid_t me = getpid();
    pid = fork();
    assert(pid >= 0);
    if (pid == 0)
        _exit(getppid() == me ? 0 : 1);
    assert(waitpid(pid, &st, 0) == pid);
    assert(WEXITSTATUS(st) == 0);

    unlink(PATH);
    printf("test_fork passed\n");
    return 0;
}
