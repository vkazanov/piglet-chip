CC = gcc
CFLAGS = -g -Wall -Wextra $(shell pkg-config --cflags libevdev)
LDFLAGS =  $(shell pkg-config --libs libevdev)

all: pchip test

pchip: main.c chip8.c key-evdev.c fb-console.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

test: test.c chip8.c key-evdev.c fb-console.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
	./test

clean:
	rm -vf pchip test

.PHONY: test all
