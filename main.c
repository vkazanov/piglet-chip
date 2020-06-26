#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "chip8.h"


int main(int argc, char *argv[])
{
    /* TODO: handle keyboard errors for keyboard errors */
    /* TODO: better makefile (handle .h changes)  */
    /* TODO: deinit things (also in tests) */
    /* TODO: sound */
    /* TODO: output in the console after running */
    /* TODO: rename things (fb-console -> display) */

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

    keyboard *key = NULL;
    int rc = keyboard_new(keyboard_path, &key);
    if (KEYBOARD_SUCCESS != rc) {
        fprintf(stderr, "Failed to init keyboard:  %s\n", keyboard_path);
        exit(EXIT_FAILURE);
    }

    fb_console *display = NULL;
    rc = fb_new(&display);
    if (rc != FB_CONSOLE_SUCCESS) {
        fprintf(stderr, "Failed to init display\n");
        exit(EXIT_FAILURE);
    }

    chip8 vm;
    chip8_reset(&vm, key, display);

    ssize_t bytes_read = read(rom_fd, vm.ram + PROGRAM_START_BYTES, (size_t)sb.st_size);
    if (bytes_read != sb.st_size) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    /*
     * main loop
     * */

    chip8_redraw(&vm);

    for (;;) {
        chip8_cpu_tick(&vm);
        chip8_timers_tick(&vm);
        chip8_redraw(&vm);

        uint32_t usec_to_next = chip8_tick(&vm);
        usleep(usec_to_next);
    }

    return 0;
}
