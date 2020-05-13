CC = gcc
CFLAGS = -std=gnu11 -O3 -g

all: pchip

pchip: main.c vm.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -vf pchip
