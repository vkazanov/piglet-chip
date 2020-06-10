#ifndef FB_CONSOLE_H
#define FB_CONSOLE_H

#include <stdint.h>
#include <stdbool.h>

#define FRAMEBUF_HEIGHT 32u
#define FRAMEBUF_WIDTH 64u
#define FRAMEBUF_SIZE (FRAMEBUF_HEIGHT * FRAMEBUF_WIDTH)
#define SPRITE_MAX_SIZE 15

typedef struct fb_console {
    /* Framebuffer, previous and new */
    uint8_t fb[FRAMEBUF_SIZE];
    uint8_t fb_old[FRAMEBUF_SIZE];

    bool is_dirty;

} fb_console;

void fb_new(fb_console **fb);

void fb_free(fb_console *fb);

void fb_draw_sprite(fb_console *fb, uint8_t *source, uint8_t bytes, uint8_t x, uint8_t y);

/* TODO: bad naming, should be something like refresh */
void fb_redraw(fb_console *fb);

void fb_clear(fb_console *fb);

#endif /* FB_CONSOLE_H */
