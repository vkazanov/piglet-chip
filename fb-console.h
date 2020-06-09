#ifndef FB_CONSOLE_H
#define FB_CONSOLE_H

#include <stdint.h>
#include <stdbool.h>

#define FRAMEBUF_HEIGHT 64
#define FRAMEBUF_WIDTH 32
#define FRAMEBUF_SIZE (FRAMEBUF_HEIGHT * FRAMEBUF_WIDTH)

typedef struct fb_console {
    /* Framebuffer, previous and new */
    uint8_t fb[FRAMEBUF_SIZE];
    uint8_t fb_old[FRAMEBUF_SIZE];

    bool is_dirty;

} fb_console;

void fb_redraw(fb_console *fb);

void fb_new(fb_console **fb);

void fb_clear(fb_console *fb);

#endif /* FB_CONSOLE_H */
