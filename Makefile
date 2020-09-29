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

extern.o: tests/tests-extern.c
	gcc -xc -c -o extern.o tests/tests-extern.c

test: mmcc extern.o
	./mmcc tests/test.c > tmp.s
	gcc -static -o tmp tmp.s extern.o
	./tmp

test-stage2: mmcc-stage2 extern.o
	./mmcc-gen2 tests/test.c > tmp.s
	gcc -static -o tmp tmp.s extern.o
	./tmp

debug: mmcc
	cp .gdbinit ~/.gdbinit

clean:
	rm -rf mmcc mmcc-gen2 *.o *~ tmp*

.PHONY: test clean
