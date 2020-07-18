CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

all: mmcc /tmp/tmpfs

/tmp/tmpfs:
	mkdir -p /tmp/tmpfs

$(OBJS): mmcc.h

mmcc: $(OBJS)
	$(CC) -o mmcc $(OBJS) $(LDFLAGS)

mmcc-stage2: mmcc $(SRCS) $(OBJS) self.sh
	./self.sh

test: mmcc
	./mmcc tests/test.c > tmp.s
	echo 'int ext1; int *ext2; int static_func() { return 5; }' | gcc -xc -c -o tmp2.o -
	gcc -static -o tmp tmp.s tmp2.o
	./tmp

test-stage2: mmcc-stage2
	./mmcc-gen2 tests/test.c > tmp.s
	echo 'int ext1; int *ext2; int static_func() { return 5; }' | gcc -xc -c -o tmp2.o -
	gcc -static -o tmp tmp.s tmp2.o
	./tmp

debug: mmcc
	cp .gdbinit ~/.gdbinit

clean:
	rm -rf mmcc mmcc-gen2 *.o *~ tmp*

.PHONY: test clean
