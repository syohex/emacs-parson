EMACS_ROOT ?= ../..
EMACS ?= emacs

CC      = gcc
LD      = gcc
CPPFLAGS = -I$(EMACS_ROOT)/src
CFLAGS = -std=gnu99 -O3 -Wall -fPIC $(CPPFLAGS)

.PHONY : test bench

all: parson-core.so

parson-core.so: parson.o parson-core.o
	$(LD) -shared $(LDFLAGS) -o $@ $^

parson-core.o: parson-core.c
	$(CC) $(CFLAGS) -c -o $@ $<

parson.o: parson.c
	$(CC) $(CFLAGS) -c -o $@ $<

bench:
	$(EMACS) -Q -batch -L . -l bench/bench.el

clean:
	-rm -f parson-core.so parson.o parson-core.o

test:
	$(EMACS) -Q -batch -L . -l test/test.el -f ert-run-tests-batch-and-exit
