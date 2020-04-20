CFLAGS=-std=c11 -g -static

mmcc: mmcc.c

test: mmcc
				./test.sh

clean:
				rm -f mmcc *.o *~ tmp*

.PHONY: test clean
