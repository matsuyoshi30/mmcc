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
	cc -static -o tmp tmp.s
	./tmp

debug: mmcc
	cp .gdbinit ~/.gdbinit

clean:
	rm -f mmcc *.o *~ tmp*

.PHONY: test clean
