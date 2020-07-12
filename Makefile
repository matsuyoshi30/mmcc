CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

all: mmcc /tmp/tmpfs

/tmp/tmpfs:
	mkdir -p /tmp/tmpfs

mmcc: $(OBJS)
	$(CC) -o mmcc $(OBJS) $(LDFLAGS)

$(OBJS): mmcc.h

test: mmcc
	./mmcc tests/test.c > tmp.s
	echo 'int static_func() { return 5; }' | gcc -xc -c -o tmp2.o -
	gcc -static -o tmp tmp.s tmp2.o
	./tmp

debug: mmcc
	cp .gdbinit ~/.gdbinit

clean:
	rm -f mmcc *.o *~ tmp*

.PHONY: test clean
