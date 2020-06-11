#include <stdio.h>
#include <termio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

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

void fb_free(fb_console *fb)
{
    if (!fb)
        return;
    free(fb);
}

void fb_draw_sprite(fb_console *fb, uint8_t *source, uint8_t bytes, uint8_t x, uint8_t y)
{
    assert(bytes <= SPRITE_MAX_SIZE);

    for (size_t byt = 0; byt < bytes; byt++ ) {
        for (size_t bt = 0; bt < 8; bt++) {
            /* make sure sprites wrap around */
            uint8_t target_y = (y + byt) % FRAMEBUF_HEIGHT;
            uint8_t target_x = (x + (7u - bt)) % FRAMEBUF_WIDTH;

            /* get the final index */
            size_t fb_idx = target_y * FRAMEBUF_WIDTH + target_x;

            fb->fb[fb_idx] = source[byt] & (1 << bt);
        }
    }

    fb->is_dirty = true;
}

void fb_redraw(fb_console *fb)
{
    write(fileno(stdout), "\033c", 4);

    putchar('*');
    for (size_t y = 0; y < FRAMEBUF_WIDTH; y++)
        putchar('-');
    putchar('*');
    putchar('\n');

    for (size_t y = 0; y < FRAMEBUF_HEIGHT; y++) {
        putchar('|');
        for (size_t x = 0; x < FRAMEBUF_WIDTH; x++) {
            size_t idx = y * FRAMEBUF_WIDTH + x;
            uint8_t pixel = fb->fb[idx];
            uint8_t pixel_old = fb->fb_old[idx];
            /* printf("%zu %zu -> %d\n", x, y, pixel); */
            /* if (pixel) */
            /*     putchar('0'); */
            /* else */
            /*     putchar(' '); */
            if (pixel != pixel_old) {
                if (pixel == 0) {
                    /* printf("pixel not set"); */
                    putchar(' ');
                } else {
                    putchar('0');
                    /* printf("found a pixel set"); */
                }
            } else {
                putchar(' ');
            }
        }
        putchar('|');
        putchar('\n');
    }
    putchar('*');
    for (size_t y = 0; y < FRAMEBUF_WIDTH; y++)
        putchar('-');
    putchar('*');
    putchar('\n');

    memcpy(fb->fb_old, fb->fb, sizeof(fb->fb));
    memset(fb->fb, 0, FRAMEBUF_SIZE);
}


void fb_clear(fb_console *fb)
{
    memset(fb->fb, 0, FRAMEBUF_SIZE);
    memset(fb->fb_old, 0, FRAMEBUF_SIZE);
    fb->is_dirty = true;
}
