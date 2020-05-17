#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <termio.h>

#include "vm.h"

#define MEMORY_SIZE_BYTES (2 << 11) /* 4K */
#define PROGRAM_START 0x200
#define MAX_STACK_DEPTH 32
/* #define FREQUENCY 10      /\* Hz *\/ */
#define FREQUENCY 60      /* Hz */
#define USECONDS_PER_STEP (1000000 / FREQUENCY) /* Seconds per step */
#define FRAMEBUF_HEIGHT 64
#define FRAMEBUF_WIDTH 32

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

    uint8_t fb[FRAMEBUF_HEIGHT * FRAMEBUF_WIDTH];
    uint8_t fb_old[FRAMEBUF_HEIGHT * FRAMEBUF_WIDTH];

    bool is_fb_dirty;
} vm = {0};

void redraw_fb(void)
{

}

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

    /* clear screen using an ANSI code */
    /* TODO: should a the part of an IO module */
    write(fileno(stdin), "\033c", 4);

    vm.PC = PROGRAM_START;
    printf("Hello, piglet!\n");

    /* setup the terminal */
    /* TODO: extract an IO handling module */
    struct termios orig_term_attr;
    struct termios new_term_attr;
    /* TODO: error checking */
    tcgetattr(fileno(stdin), &orig_term_attr);

    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;

    /* TODO: error checking */
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* main loop */
    while (state == RUNNING) {
        /* (void)write(STDOUT_FILENO, "tick\n", 5); */

        struct timeval start_time, end_time;
        gettimeofday(&(start_time), NULL);

        usleep(1000);

        /* big-endian (MSB first) */
        uint16_t instruction = vm.ram[vm.PC] << 8;
        instruction |= vm.ram[vm.PC + 1];
        vm.PC += 2;

        /* decrease the timer */
        if (vm.DT > 0) {
            vm.DT -= 1;
        }

        /* decrease the sound timer */
        if (vm.ST > 0) {
            vm.ST -= 1;
        }

        /* refresh the image */
        if (vm.is_fb_dirty) {
            redraw_fb();
        }

        /* read input */
        int c = fgetc(stdin);
        if (c != EOF)
            fputc(c,stdout);

        /* TODO: log */
        /* TODO: exec */

        gettimeofday(&(end_time), NULL);
        useconds_t step_took_useconds =
            ((end_time).tv_sec * 1000000L + (end_time).tv_usec) -
            ((start_time).tv_sec * 1000000L + (start_time).tv_usec);

        /* printf("step took: %d usec\n", step_took_useconds); */
        /* printf("time to sleep: %d usec\n", USECONDS_PER_STEP - step_took_useconds); */
        usleep(USECONDS_PER_STEP - step_took_useconds);
    }

    return 0;
}
