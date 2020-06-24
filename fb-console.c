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

void fb_draw_sprite(fb_console *fb, uint8_t *source, uint8_t bytes, uint8_t x, uint8_t y, bool *is_pixel_erased)
{
    assert(bytes <= SPRITE_MAX_SIZE);

    *is_pixel_erased = false;

    for (size_t byt = 0; byt < bytes; byt++ ) {
        for (size_t bt = 0; bt < 8; bt++) {
            /* make sure sprites wrap around */
            uint8_t target_y = (y + byt) % FRAMEBUF_HEIGHT;
            uint8_t target_x = (x + (7u - bt)) % FRAMEBUF_WIDTH;

            /* get the final pixel index */
            size_t fb_idx = target_y * FRAMEBUF_WIDTH + target_x;

            /* XOR the pixel onto the screen */
            uint8_t sprite_pixel = !!(source[byt] & (1 << bt));
            uint8_t target_pixel = !!(fb->fb[fb_idx]);
            fb->fb[fb_idx] = sprite_pixel ^ target_pixel;

            /* Check if a pixel was erased */
            if (target_pixel && !fb->fb[fb_idx])
                *is_pixel_erased = true;
        }
    }

    fb->is_dirty = true;
}

void fb_redraw(fb_console *fb, bool keyboard_state[16])
{
    if (!fb->is_dirty)
        return;
    fb->is_dirty = false;


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
            if (pixel)
                putchar('0');
            else
                putchar(' ');
        }
        putchar('|');
        putchar('\n');
    }
    putchar('*');
    for (size_t y = 0; y < FRAMEBUF_WIDTH; y++)
        putchar('-');
    putchar('*');
    putchar('\n');

    /* TODO: cleaner  */
    if (keyboard_state[0x1]) printf("%.1X", 0x1); else printf(" ");
    if (keyboard_state[0x2]) printf("%.1X", 0x2); else printf(" ");
    if (keyboard_state[0x3]) printf("%.1X", 0x3); else printf(" ");
    if (keyboard_state[0xC]) printf("%.1X", 0xC); else printf(" ");
    putchar('\n');
    if (keyboard_state[0x4]) printf("%.1X", 0x4); else printf(" ");
    if (keyboard_state[0x5]) printf("%.1X", 0x5); else printf(" ");
    if (keyboard_state[0x6]) printf("%.1X", 0x6); else printf(" ");
    if (keyboard_state[0xD]) printf("%.1X", 0xD); else printf(" ");
    putchar('\n');
    if (keyboard_state[0x7]) printf("%.1X", 0x7); else printf(" ");
    if (keyboard_state[0x8]) printf("%.1X", 0x8); else printf(" ");
    if (keyboard_state[0x9]) printf("%.1X", 0x9); else printf(" ");
    if (keyboard_state[0xE]) printf("%.1X", 0xE); else printf(" ");
    putchar('\n');
    if (keyboard_state[0xA]) printf("%.1X", 0xA); else printf(" ");
    if (keyboard_state[0x0]) printf("%.1X", 0x0); else printf(" ");
    if (keyboard_state[0xB]) printf("%.1X", 0xB); else printf(" ");
    if (keyboard_state[0xF]) printf("%.1X", 0xF); else printf(" ");
    putchar('\n');

    fb->is_dirty = false;
}


void fb_clear(fb_console *fb)
{
    memset(fb->fb, 0, FRAMEBUF_SIZE);

    fb->is_dirty = true;
}
