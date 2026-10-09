#pragma once

// раскладка struct sigaction, которую ждёт ядро на x86_64
struct k_sigaction {
    void (*handler)(int);
    unsigned long flags;
    void (*restorer)(void);
    unsigned long mask;
};

#define K_SA_RESTORER 0x04000000
