CC = gcc
CFLAGS = -g -Wall -Wextra $(shell pkg-config --cflags libevdev)
LDFLAGS =  $(shell pkg-config --libs libevdev)

all: pchip pchip-test

pchip: main.c chip8.c keyboard.c fb-console.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

pchip-test: test.c chip8.c keyboard.c fb-console.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test: pchip-test
	./$<

clean:
	rm -vf pchip test

.PHONY: test all
