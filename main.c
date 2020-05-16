#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdint.h>

#include "vm.h"

#define MEMORY_SIZE_BYTES (2 << 11) /* 4K */
#define PROGRAM_START 0x200
#define MAX_STACK_DEPTH 32
#define FREQUENCY 1      /* Hz */
/* #define FREQUENCY 60      /\* Hz *\/ */
#define USECONDS_PER_STEP (1000000 / FREQUENCY) /* Seconds per step */

enum {
    RUNNING,
    PAUSED,
} state;

struct {
    uint8_t V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF;

    uint8_t I;
    uint16_t PC;
    uint8_t DT;
    uint8_t ST;

    uint8_t ram[MEMORY_SIZE_BYTES];
    uint8_t stack[MAX_STACK_DEPTH];
} vm = {0};

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

    if (sb.st_size > MEMORY_SIZE_BYTES) {
        fprintf(stderr, "Too big to load:  %s\n", path);
        exit(EXIT_FAILURE);
    }

    int rom_fd = open(path, O_RDONLY);
    if (rom_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (read(rom_fd, vm.ram, MEMORY_SIZE_BYTES) < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    /* TODO: How big should vm.PC be? */
    vm.PC = PROGRAM_START;
    printf("Hello, piglet!\n");

    while (state == RUNNING) {
        struct timeval start_time, end_time;
        gettimeofday(&(start_time), NULL);

        (void)write(STDOUT_FILENO, "tick\n", 5);
        usleep(500000);

        /* big-endian (MSB first) */
        uint16_t instruction = vm.ram[vm.PC] << 8;
        instruction |= vm.ram[vm.PC + 1];

        vm.PC += 2;
        /* TODO: log */
        /* TODO: exec */


        gettimeofday(&(end_time), NULL);
        useconds_t step_took_useconds =
            ((end_time).tv_sec * 1000000L + (end_time).tv_usec) -
            ((start_time).tv_sec * 1000000L + (start_time).tv_usec);

        usleep(USECONDS_PER_STEP - step_took_useconds);
    }

    return 0;
}
