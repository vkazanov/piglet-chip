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
#include <time.h>

#include "chip8.h"

void fb_redraw(chip8 *vm)
{
    for (size_t row = 0; row < FRAMEBUF_HEIGHT; row++) {
        for (size_t col = 0; col < FRAMEBUF_WIDTH; col++) {
            size_t idx = row * FRAMEBUF_WIDTH + col;
            uint8_t pixel = vm->fb[idx];
            uint8_t pixel_old = vm->fb_old[idx];
            if (pixel != pixel_old) {
                if (pixel == 0) {
                    putchar(' ');
                } else {
                    putchar('O');
                }
            }
        }
        putchar('\n');
    }

    memcpy(vm->fb_old, vm->fb, sizeof(vm->fb));
}

void fb_init(void)
{
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
}

void fb_clear(void)
{
    write(fileno(stdin), "\033c", 4);
    for (size_t row = 0; row < FRAMEBUF_HEIGHT; row++) {
        for (size_t col = 0; col < FRAMEBUF_WIDTH; col++) {
            putchar(' ');
        }
        putchar('\n');
    }
}

void step(uint16_t instruction)
{
}

int main(int argc, char *argv[])
{
    (void)argc; (void) argv;

    /* TODO: integrate the evdev-based keyboard module */
    /* TODO: implement the keyboard ops using the module */
    /* TODO: framebuf-related ops */
    /* TODO: preloaded images (numbers and stuff) */
    /* TODO: handle keyboard errors for keyboard errors */
    /* TODO: key and key value constants for key-evdev (reuse in key-related
     * tests) */
    /* TODO: main interpreter loop */

    if (argc != 3){
        fprintf(stderr, "Usage: %s <path/to/rom> <path/to/keyboard/dev>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *rom_path = argv[1];
    const char *keyboard_path = argv[2];

    struct stat sb;
    if (stat(rom_path, &sb) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }

    if ((sb.st_mode & S_IFMT) != S_IFREG) {
        fprintf(stderr, "File expected: %s\n", rom_path);
        exit(EXIT_FAILURE);
    }

    if (sb.st_size > MEMORY_SIZE_BYTES) {
        fprintf(stderr, "Too big to load:  %s\n", rom_path);
        exit(EXIT_FAILURE);
    }

    int rom_fd = open(rom_path, O_RDONLY);
    if (rom_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }



    chip8 vm;
    chip8_reset(&vm);
    if (read(rom_fd, vm.ram, MEMORY_SIZE_BYTES) < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    /* random number generator */
    srand(time(NULL));

    printf("Hello, piglet!\n");

    fb_init();
    fb_clear();

    /* main loop */
    while (state == RUNNING) {
        /* (void)write(STDOUT_FILENO, "tick\n", 5); */

        struct timeval start_time, end_time;
        gettimeofday(&(start_time), NULL);

        /* TODO: tmp */
        usleep(1000);

        uint16_t instruction = chip8_fetch(&vm);
        chip8_exec(&vm, instruction);
        chip8_bump_PC(&vm);

        /* TODO: should be a step */
        /* decrease the timer */
        if (vm.DT > 0) {
            vm.DT -= 1;
        }

        /* decrease the sound timer */
        if (vm.ST > 0) {
            vm.ST -= 1;
            /* TODO: Play sound */
        }

        /* refresh the image */
        if (vm.is_fb_dirty) {
            fb_redraw(&vm);
            vm.is_fb_dirty = false;
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
