#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE_BYTES (2 << 11) /* 4K */
#define PROGRAM_START 0x200
#define MAX_STACK_DEPTH 32
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

typedef struct chip8 {
    uint8_t V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF;

    uint8_t I;
    uint16_t PC;
    uint8_t DT;
    uint8_t ST;

    uint8_t ram[MEMORY_SIZE_BYTES];
    uint8_t stack[MAX_STACK_DEPTH];

    uint8_t fb[FRAMEBUF_SIZE];
    uint8_t fb_old[FRAMEBUF_SIZE];

    bool is_fb_dirty;
} chip8;

void chip8_reset(chip8 *vm);

uint16_t chip8_fetch(chip8 *vm);

void chip8_bump_PC(chip8 *vm);

void chip8_exec(chip8 *vm, uint16_t instruction);

#endif /* CHIP8_H */
