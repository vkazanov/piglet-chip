#include <stdio.h>
#include <termio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fb-console.h"

void fb_new(fb_console **fb)
{
    *fb = calloc(1, sizeof(**fb));

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


void fb_redraw(fb_console *fb)
{
    for (size_t row = 0; row < FRAMEBUF_HEIGHT; row++) {
        for (size_t col = 0; col < FRAMEBUF_WIDTH; col++) {
            size_t idx = row * FRAMEBUF_WIDTH + col;
            uint8_t pixel = fb->fb[idx];
            uint8_t pixel_old = fb->fb_old[idx];
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

    memcpy(fb->fb_old, fb->fb, sizeof(fb->fb));
}


void fb_clear(fb_console *fb)
{
    (void) fb;

    write(fileno(stdin), "\033c", 4);
    for (size_t row = 0; row < FRAMEBUF_HEIGHT; row++) {
        for (size_t col = 0; col < FRAMEBUF_WIDTH; col++) {
            putchar(' ');
        }
        putchar('\n');
    }
}
