#include <stdlib.h>
#include <unistd.h>
#include <libc.h>
#include <pthread_impl.h>

char **environ;

#define AT_PHDR 3
#define AT_PHENT 4
#define AT_PHNUM 5

__attribute__((noreturn)) void __libc_start_main(long argc, char **argv)
{
    char **envp = argv + argc + 1;
    char **e = envp;
    while (*e)
        e++;
    unsigned long *auxv = (unsigned long *)(e + 1);

    unsigned long phdr = 0, phent = 0, phnum = 0;
    for (unsigned long *a = auxv; a[0]; a += 2) {
        if (a[0] == AT_PHDR)
            phdr = a[1];
        else if (a[0] == AT_PHENT)
            phent = a[1];
        else if (a[0] == AT_PHNUM)
            phnum = a[1];
    }

    environ = envp;
    __init_tls((unsigned long *)phdr, (int)phnum, (int)phent);
    __stdio_init();

    extern int main(int, char **, char **);
    exit(main((int)argc, argv, environ));
}
