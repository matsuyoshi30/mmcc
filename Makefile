CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

mmcc: $(OBJS)
	$(CC) -o mmcc $(OBJS) $(LDFLAGS)

$(OBJS): mmcc.h

test: mmcc
	./test.sh

debug: mmcc
	cp .gdbinit ~/.gdbinit

clean:
	rm -f mmcc *.o *~ tmp*

.PHONY: test clean
