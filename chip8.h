#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "key-evdev.h"
#include "fb-console.h"

/* #define DEBUG_TRACE */

#define MEMORY_SIZE_BYTES (1 << 12) /* 4K */
#define PROGRAM_START_BYTES (0x200)   /* 512 */
#define MAX_ROM_SIZE_BYTES (MEMORY_SIZE_BYTES - PROGRAM_START_BYTES)
#define MAX_STACK_DEPTH 16

#define FREQUENCY_CPU 500      /* Hz */
#define FREQUENCY_TIMER 60     /* Hz */
static_assert(FREQUENCY_CPU > FREQUENCY_TIMER,
              "CPU is expected to be faster than both DT and ST");

#define USECONDS_PER_SECOND 1000000
#define USECONDS_PER_STEP_TIMER (USECONDS_PER_SECOND / FREQUENCY_TIMER)  /* Seconds per step */
#define USECONDS_PER_STEP_CPU (USECONDS_PER_SECOND / FREQUENCY_CPU)  /* Seconds per step */

enum {
    RUNNING,
    PAUSED,
} state;

enum {
    V0, V1,V2, V3, V4, V5, V6, V7, V8, V9, Va, Vb, Vc, Vd, Ve,
    Vf
} reg_names;

typedef struct chip8 {
    /* Microseconds left to next CPU/DT/ST ticks */
    uint32_t usec_to_cpu_tick;
    uint32_t usec_to_timer_tick;

    /* 0x0..0xE - general purpose registers, 0xF for flags  */
    uint8_t regs[0xf];

    /* Mostly for memory addresses, only 12 lower bits used */
    uint16_t I;
    /* Program counter */
    uint16_t PC;

    /* Delay and sound timers, 60 Hz */
    uint8_t DT;
    uint8_t ST;

    /* Stack pointer */
    uint8_t SP;
    /* Stack */
    uint16_t stack[MAX_STACK_DEPTH];

    /* Memory */
    uint8_t ram[MEMORY_SIZE_BYTES];

    /* IO */
    fb_console *display;
    key_evdev *keyboard;
} chip8;

void chip8_reset(chip8 *vm, key_evdev *keyboard, fb_console *display);

uint16_t chip8_fetch(chip8 *vm);

void chip8_exec(chip8 *vm, uint16_t instruction);

void chip8_redraw(chip8 *vm);

void chip8_timers(chip8 *vm);

#endif /* CHIP8_H */
