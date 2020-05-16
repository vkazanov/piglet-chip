#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#include "vm.h"

#define PCHIP_MEMORY_SIZE_BYTES (2 << 11) /* 4K */

int main(int argc, char *argv[])
{
    (void)argc; (void) argv;

    if (argc != 2){
        fprintf(stderr, "Usage: %s <path/to/rom>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *path = argv[1];

    struct stat sb;
    if (stat(path, &sb) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }

    if ((sb.st_mode & S_IFMT) != S_IFREG) {
        fprintf(stderr, "File expected: %s\n", path);
        exit(EXIT_FAILURE);
    }

    if (sb.st_size > PCHIP_MEMORY_SIZE_BYTES) {
        fprintf(stderr, "Too big to load:  %s\n", path);
        exit(EXIT_FAILURE);
    }

    int rom_fd = open(path, O_RDONLY);
    if (rom_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    uint8_t rom[PCHIP_MEMORY_SIZE_BYTES] = {0};
    if (read(rom_fd, rom, PCHIP_MEMORY_SIZE_BYTES) < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    printf("Hello, piglet!\n");

    return 0;
}
