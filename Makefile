CC = gcc
CFLAGS = -std=gnu11 -O3 -g

all: pchip test

pchip: main.c chip8.c
	$(CC) $(CFLAGS) $^ -o $@

test: test.c chip8.c
	$(CC) $(CFLAGS) $^ -o $@
	./test

clean:
	rm -vf pchip test
