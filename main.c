#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "chip8.h"


int main(int argc, char *argv[])
{
    (void)argc; (void) argv;

    /* TODO: instruction logging */
    /* TODO: fix test game BLINKY */
    /* TODO: handle keyboard errors for keyboard errors */
    /* TODO: key and key value constants for key-evdev (reuse in key-related
     * tests) */
    /* TODO: main loop: fix the loop itself */
    /* TODO: main loop: fix the timers */
    /* TODO: main loop: fix the running state (do i need it at all?)  */
    /* TODO: main loop: add pause  */
    /* TODO: sound */

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
        fprintf(stderr, "ROM file expected: %s\n", rom_path);
        exit(EXIT_FAILURE);
    }

    if (sb.st_size > MAX_ROM_SIZE_BYTES) {
        fprintf(stderr, "Too big to load:  %s\n", rom_path);
        exit(EXIT_FAILURE);
    }

    int rom_fd = open(rom_path, O_RDONLY);
    if (rom_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    /* TODO: SIGSEGV on wrong devices */
    key_evdev *keyboard = NULL;
    int rc = key_evdev_new(keyboard_path, &keyboard);
    if (KEY_EVDEV_SUCCESS != rc) {
        fprintf(stderr, "Failed to init keyboard:  %s\n", keyboard_path);
        exit(EXIT_FAILURE);
    }

    /* TODO: err code checks */
    fb_console *display = NULL;
    fb_new(&display);


    chip8 vm;
    chip8_reset(&vm, keyboard, display);

    ssize_t bytes_read = read(rom_fd, vm.ram + PROGRAM_START_BYTES, (size_t)sb.st_size);
    if (bytes_read != sb.st_size) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    /* TODO: move random number generator into reset code */
    srand(time(NULL));

    /* main loop */
    while (state == RUNNING) {

        struct timeval start_time, end_time;
        gettimeofday(&(start_time), NULL);

        /* TMP: */
        /* usleep(1000); */

        uint16_t instruction = chip8_fetch(&vm);
#ifdef DEBUG_TRACE
        fprintf(stderr, "PC=0x%.3x\n", vm.PC);
#endif
        chip8_exec(&vm, instruction);
        chip8_redraw(&vm);
        chip8_tick_timers(&vm);

        gettimeofday(&(end_time), NULL);
        useconds_t step_took_useconds =
            ((end_time).tv_sec * 1000000L + (end_time).tv_usec) -
            ((start_time).tv_sec * 1000000L + (start_time).tv_usec);

        /* printf("step took: %d usec\n", step_took_useconds); */
        /* printf("time to sleep: %d usec\n", USECONDS_PER_STEP - step_took_useconds); */
        usleep(USECONDS_PER_STEP - step_took_useconds);
    }

    /* TODO: deinit things */

    return 0;
}
