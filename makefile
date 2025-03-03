.PHONY: run

run:
	docker run --rm -it -v $(shell pwd):/home/user compilerbook

build:
	docker build -t compilerbook .

CFLAGS=-std=c11 -g -static

9cc: 9cc.c

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean
