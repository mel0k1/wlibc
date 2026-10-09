#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *getenv(const char *name)
{
    if (!environ)
        return 0;
    size_t l = strlen(name);
    for (char **e = environ; *e; e++)
        if (!strncmp(*e, name, l) && (*e)[l] == '=')
            return *e + l + 1;
    return 0;
}
