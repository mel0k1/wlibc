#include <stdlib.h>
#include <unistd.h>

char **environ __attribute__((weak));

__attribute__((noreturn)) void __libc_start_main(long argc, char **argv)
{
    environ = argv + argc + 1;
    extern int main(int, char **, char **);
    exit(main((int)argc, argv, environ));
}
