#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE_BYTES (2 << 11) /* 4K */
#define PROGRAM_START 0x200
#define MAX_STACK_DEPTH 16
/* #define FREQUENCY 10      /\* Hz *\/ */
#define FREQUENCY 60      /* Hz */
#define USECONDS_PER_STEP (1000000 / FREQUENCY) /* Seconds per step */
#define FRAMEBUF_HEIGHT 64
#define FRAMEBUF_WIDTH 32
#define FRAMEBUF_SIZE (FRAMEBUF_HEIGHT * FRAMEBUF_WIDTH)

enum {
    RUNNING,
    PAUSED,
} state;

enum {
    V0, V1,V2, V3, V4, V5, V6, V7, V8, V9, Va, Vb, Vc, Vd, Ve,
    Vf
} reg_names;

typedef struct chip8 {
    /* 0x0..0xE - general purpose registers, 0xF for flags  */
    uint8_t regs[0xf];

    /* Mostly for memory addresses, only 12 lower bits used */
    uint16_t I;
    /* Program counter */
    uint16_t PC;

    /* Delay timer, 60 Hz */
    uint8_t DT;
    /* Sound timer, 60 Hz */
    uint8_t ST;

    /* Stack pointer */
    uint8_t SP;
    /* Stack */
    uint16_t stack[MAX_STACK_DEPTH];

    /* Memory */
    uint8_t ram[MEMORY_SIZE_BYTES];

    /* Framebuffer, previous and new */
    uint8_t fb[FRAMEBUF_SIZE];
    uint8_t fb_old[FRAMEBUF_SIZE];

    bool is_fb_dirty;
} chip8;

void chip8_reset(chip8 *vm);

uint16_t chip8_fetch(chip8 *vm);

void chip8_bump_PC(chip8 *vm);

void chip8_exec(chip8 *vm, uint16_t instruction);

#endif /* CHIP8_H */
