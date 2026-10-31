# Build the reproducers for the host and for an aarch64 guest.
#
#   make                 # host binaries
#   make CC=aarch64-linux-gnu-gcc PREFIX=aarch64-   # guest binaries

CC      ?= cc
PREFIX  ?=
CFLAGS  ?= -O0 -static -Wall

all: $(PREFIX)getrandom-zero-len $(PREFIX)null-out-buffer

$(PREFIX)getrandom-zero-len: getrandom-zero-len.c
	$(CC) $(CFLAGS) -o $@ $<

$(PREFIX)null-out-buffer: null-out-buffer.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PREFIX)getrandom-zero-len $(PREFIX)null-out-buffer

.PHONY: all clean
