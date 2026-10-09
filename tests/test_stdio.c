#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PATH "/tmp/wlibc_stdio_test.dat"

int main(void)
{
    const char *msg = "first line\nsecond line\nthird line\n";
    size_t msglen = strlen(msg);

    // запись с буферизацией, объём больше буфера
    FILE *f = fopen(PATH, "w");
    assert(f != NULL);
    assert(fputs(msg, f) == 0);
    char big[4096];
    memset(big, 'x', sizeof(big));
    assert(fwrite(big, 1, sizeof(big), f) == sizeof(big));
    assert(fprintf(f, "num=%d hex=%#x str=%s\n", 42, 255, "end") > 0);
    assert(fclose(f) == 0);

    // чтение целиком
    f = fopen(PATH, "r");
    assert(f != NULL);
    char rbuf[8192];
    size_t n = fread(rbuf, 1, sizeof(rbuf), f);
    assert(n == msglen + sizeof(big) + 24);
    assert(memcmp(rbuf, msg, msglen) == 0);
    assert(memcmp(rbuf + msglen, big, sizeof(big)) == 0);
    assert(fgetc(f) == EOF);
    assert(feof(f));
    assert(fclose(f) == 0);

    // fgets построчно
    f = fopen(PATH, "r");
    assert(f != NULL);
    char line[128];
    assert(fgets(line, sizeof(line), f) != NULL);
    assert(strcmp(line, "first line\n") == 0);
    assert(fgets(line, sizeof(line), f) != NULL);
    assert(strcmp(line, "second line\n") == 0);
    assert(fgets(line, sizeof(line), f) != NULL);
    assert(strcmp(line, "third line\n") == 0);
    // дальше идут 4096 'x' — fgets вернёт полную строку без '\n'
    assert(fgets(line, sizeof(line), f) != NULL);
    assert(strlen(line) == 127);
    assert(fseek(f, 0, SEEK_END) == 0);
    assert(fgets(line, sizeof(line), f) == NULL);
    assert(feof(f));
    assert(fclose(f) == 0);

    // fseek/ftell/ungetc
    f = fopen(PATH, "r");
    assert(f != NULL);
    assert(ftell(f) == 0);
    assert(fgetc(f) == 'f');
    assert(fgetc(f) == 'i');
    assert(ftell(f) == 2);
    assert(fseek(f, 0, SEEK_SET) == 0);
    assert(ftell(f) == 0);
    assert(fgetc(f) == 'f');
    assert(ungetc('Z', f) == 'Z');
    assert(fgetc(f) == 'Z');
    assert(fgetc(f) == 'i');
    assert(fseek(f, -1, SEEK_END) == 0);
    assert(fgetc(f) == '\n');
    assert(fgetc(f) == EOF);
    assert(fclose(f) == 0);

    // посимвольный вывод и чтение
    f = fopen(PATH, "w");
    assert(f != NULL);
    assert(fputc('a', f) == 'a');
    assert(putc('b', f) == 'b');
    assert(fputc('c', f) == 'c');
    assert(fclose(f) == 0);
    f = fopen(PATH, "r");
    assert(fgetc(f) == 'a');
    assert(getc(f) == 'b');
    assert(fgetc(f) == 'c');
    assert(fgetc(f) == EOF);
    assert(fclose(f) == 0);

    // append-режим
    f = fopen(PATH, "a");
    assert(f != NULL);
    assert(fputs("appended\n", f) >= 0);
    assert(fclose(f) == 0);
    f = fopen(PATH, "r");
    char tail[32];
    assert(fseek(f, -9, SEEK_END) == 0);
    assert(fgets(tail, sizeof(tail), f) != NULL);
    assert(strcmp(tail, "appended\n") == 0);
    assert(fclose(f) == 0);

    // fdopen + fflush
    int fds[2];
    assert(pipe(fds) == 0);
    FILE *wf = fdopen(fds[1], "w");
    FILE *rf = fdopen(fds[0], "r");
    assert(wf != NULL && rf != NULL);
    assert(fputs("via pipe\n", wf) >= 0);
    assert(fflush(wf) == 0); // без fflush строка осталась бы в буфере
    char pline[32];
    assert(fgets(pline, sizeof(pline), rf) != NULL);
    assert(strcmp(pline, "via pipe\n") == 0);
    assert(fclose(rf) == 0);
    assert(fclose(wf) == 0);

    // snprintf/sprintf через буфер не сломались
    char sb[64];
    assert(snprintf(sb, sizeof(sb), "%d/%s", 5, "ok") == 4);
    assert(strcmp(sb, "5/ok") == 0);

    // printf уже проверен: вывод выше через stdout-поток
    printf("test_stdio passed\n");
    unlink(PATH);
    return 0;
}
