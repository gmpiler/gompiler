CFLAGS=-std=c11 -g -static

gompiler: gompiler.c

test: gompiler
		./test.sh

clean:
		rm -f gompiler *.o *~ tmp*

.PHONY: test clean
