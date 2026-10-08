ARCH := x86_64
SYS := linux

CC := gcc
AR := ar

CFLAGS := -std=c11 -O2 -g -Wall -Wextra -ffreestanding -fno-builtin \
          -fno-stack-protector -fno-asynchronous-unwind-tables -nostdinc
CPPFLAGS := -Iinclude -Isrc/internal -Isysdeps/$(SYS)/$(ARCH)
LDFLAGS := -static -nostdlib

SRCS := $(wildcard src/*/*.c)
OBJS := $(SRCS:.c=.o)
CRT := obj/crt1.o
LIBC := libc.a

TESTS_SRC := $(wildcard tests/*.c)
TESTS := $(patsubst tests/%.c,bin/%,$(TESTS_SRC))

all: $(LIBC) $(CRT)

$(CRT): sysdeps/$(SYS)/$(ARCH)/crt1.s
	@mkdir -p $(@D)
	$(CC) -c $< -o $@

%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(LIBC): $(OBJS)
	$(AR) rcs $@ $^

bin/%: tests/%.c $(LIBC) $(CRT)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o bin/$*.o
	$(CC) $(LDFLAGS) -o $@ bin/$*.o $(CRT) $(LIBC)

test: $(TESTS)
	@set -e; for t in $(TESTS); do ./$$t; done; echo "ALL TESTS PASSED"

clean:
	rm -rf obj bin $(LIBC)

.PHONY: all test clean
