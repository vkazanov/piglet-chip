#include "chip8.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define MIN(a,b)                                \
    ({ __auto_type _a = (a);                    \
        __auto_type _b = (b);                   \
        _a < _b ? _a : _b; })

static void load_sprites(chip8 *vm);

void chip8_reset(chip8 *vm, key_evdev *keyboard, fb_console *display)
{
    *vm = (chip8){0};
    vm->PC = PROGRAM_START_BYTES;
    vm->keyboard = keyboard;
    vm->display = display;
    assert(KEY_EVDEV_SUCCESS == key_evdev_flush(vm->keyboard));

    load_sprites(vm);

    srand(time(NULL));
}

const uint8_t sprites[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

static void load_sprites(chip8 *vm)
{
    memcpy(&vm->ram[0x00], &sprites[0], sizeof(sprites) / sizeof(sprites[0]));
}

uint16_t chip8_fetch(chip8 *vm)
{
    /* big-endian (MSB first) */

    uint16_t instruction = vm->ram[vm->PC] << 8;
    instruction |= vm->ram[vm->PC + 1];
    return instruction;
}

void chip8_exec(chip8 *vm, uint16_t instruction)
{
    uint16_t type = (0xF000 & instruction) >> 12;
    uint16_t x = (0x0F00 & instruction) >> 8;
    uint16_t y = (0x00F0 & instruction) >> 4;
    uint16_t nnn = (0x0FFF & instruction);
    uint16_t kk = (0x00FF & instruction);
    uint16_t n = (0x000F & instruction);

    bool do_step = true;

    switch (type) {
    case 0x0:{
        switch (nnn) {
        case 0x0e0:{
            /* 00e0 - CLS */
            /* Clear screen */
#ifdef DEBUG_TRACE
            fprintf(stderr, "CLS\n");
#endif
            fb_clear(vm->display);
            break;
        }
        case 0x0ee:{
            /* 00e0 - RET */
            /* Return from a subroutine */
#ifdef DEBUG_TRACE
            fprintf(stderr, "RET\n");
#endif

            vm->SP--;
            vm->PC = vm->stack[vm->SP];
            do_step = false;
            break;
        }
        default:{
            /* 0nnn - SYS addr */
            /* Jump to a routine at nnn (currently ignored) */
#ifdef DEBUG_TRACE
            fprintf(stderr, "SYS\n");
#endif

            break;
        }
        }
        break;
    }
    case 0x1:{
        /* 0x1nnn - JP addr */
        /* Jump to addr */
#ifdef DEBUG_TRACE
        fprintf(stderr, "JP 0x%.3X\n", nnn);
#endif

        vm->PC = nnn;
        do_step = false;
        break;
    }
    case 0x2:{
        /* 0x2nnn - CALL addr */
        /* A subroutine call at addr */
#ifdef DEBUG_TRACE
        fprintf(stderr, "CALL %.3X\n", nnn);
#endif

        vm->stack[vm->SP] = vm->PC + 2;
        vm->SP++;
        vm->PC = nnn;
        do_step = false;
        break;
    }
    case 0x3:{
        /* 0x3xkk - SE Vx, byte */
        /* Compare value in Vx with byte kk, skip instr if equal */
#ifdef DEBUG_TRACE
        fprintf(stderr, "SE V%X, 0x%.2X\n", x, kk);
#endif

        if (vm->regs[x] == kk) {
            vm->PC += 2;
        }
        break;
    }
    case 0x4:{
        /* 0x4xkk - SNE Vx, byte */
        /* Compare value in Vx with byte kk, skip instr if NOT equal */
#ifdef DEBUG_TRACE
        fprintf(stderr, "SNE V%.1X, 0x%.2X\n", x, kk);
#endif

        if (vm->regs[x] != kk) {
            vm->PC += 2;
        }
        break;
    }
    case 0x5:{
        /* 0x5xy0 - SE Vx, Vy */
        /* Compare value in Vx with value in Vy, skip instr if equal */
#ifdef DEBUG_TRACE
        fprintf(stderr, "SE\n");
#endif

        if (vm->regs[x] == vm->regs[y]) {
            vm->PC += 2;
        }
        break;
    }
    case 0x6:{
        /* 0x6xkk - LD Vx, byte */
        /* Load kk into Vx */
#ifdef DEBUG_TRACE
        fprintf(stderr, "LD V%.1X, 0x%.2X\n", x, kk);
#endif

        vm->regs[x] = kk;
        break;
    }
    case 0x7:{
        /* 0x7xkk - ADD Vx, byte */
        /* Add kk to the value in Vx */
#ifdef DEBUG_TRACE
        fprintf(stderr, "ADD\n");
#endif

        vm->regs[x] += kk;
        break;
    }
    case 0x8:{
        switch (n) {
        case 0x0:{
            /* 0x8xy0 - LD Vx, Vy */
            /* Load Vy into Vx */
#ifdef DEBUG_TRACE
            fprintf(stderr, "LD V%.1X, V%.1X (%u %u)\n",
                    x, y, vm->regs[x], vm->regs[y]);
#endif

            vm->regs[x] = vm->regs[y];
            break;
        }
        case 0x1:{
            /* 0x8xy1 - OR Vx, Vy */
            /* OR Vy into Vx */
#ifdef DEBUG_TRACE
            fprintf(stderr, "OR\n");
#endif

            vm->regs[x] |= vm->regs[y];
            break;
        }
        case 0x2:{
            /* 0x8xy2 - AND Vx, Vy */
            /* AND Vy into Vx */
#ifdef DEBUG_TRACE
            fprintf(stderr, "AND\n");
#endif

            vm->regs[x] &= vm->regs[y];
            break;
        }
        case 0x3:{
            /* 0x8xy3 - XOR Vx, Vy */
            /* XOR Vy into Vx */
#ifdef DEBUG_TRACE
            fprintf(stderr, "XOR\n");
#endif

            vm->regs[x] ^= vm->regs[y];
            break;
        }
        case 0x4:{
            /* 0x8xy4 - ADD Vx, Vy */
            /* ADD Vy into Vx, with carry to VF */
#ifdef DEBUG_TRACE
            fprintf(stderr, "ADD V%.1X, V%.1X\n", x, y);
#endif

            uint16_t acc = vm->regs[x] + vm->regs[y];
            vm->regs[x] = acc & 0xff;
            vm->regs[Vf] = (acc & 0xf00) >> 8;
            break;
        }
        case 0x5:{
            /* 0x8xy5 - SUB Vx, Vy */
            /* SUB Vy from Vx, with NOT borrow result to VF */

            vm->regs[Vf] = vm->regs[x] >= vm->regs[y];
            vm->regs[x] = vm->regs[x] - vm->regs[y];
#ifdef DEBUG_TRACE
            fprintf(stderr, "SUB V%.1X, V%.1X (%u %u %u)\n",
                    x, y, vm->regs[x], vm->regs[y], vm->regs[Vf]);
#endif
            break;
        }
        case 0x6:{
            /* 0x8xy6 - SHR Vx */
            /* SHR shift Vx right, if the shifted bit was 1 - set VF to 1,
             * otherwise - to 0 */
#ifdef DEBUG_TRACE
            fprintf(stderr, "SHR\n");
#endif

            vm->regs[Vf] = vm->regs[x] & 0x1;
            vm->regs[x] >>= 1;
            break;
        }
        case 0x7:{
            /* 0x8xy7 - SUBN Vx, Vy */
            /* SUB Vx from Vy, with NOT borrow result to VF,
             * otherwise - to 0 */
#ifdef DEBUG_TRACE
            fprintf(stderr, "SUBN\n");
#endif

            vm->regs[Vf] = vm->regs[y] >= vm->regs[x];
            vm->regs[x] = vm->regs[y] - vm->regs[x];
            break;
        }
        case 0xe:{
            /* 0x8xye - SHL Vx */
            /* SHR shift Vx left, if the shifted bit was 1 - set VF to 1,
             * otherwise - to 0 */
#ifdef DEBUG_TRACE
            fprintf(stderr, "SHL\n");
#endif

            vm->regs[Vf] = !!(vm->regs[x] & (0x1 << 7));
            vm->regs[x] <<= 1;
            break;
        }
        default:{
            fprintf(stderr, "Unknown inner instruction: 0x%04x\n", instruction);
            exit(EXIT_FAILURE);
        }
        }
        break;
    }
    case 0x9:{
        /* 0x9xy0 - SNE Vx, Vy */
        /* Compare Vx, Vy, if not equal - increase PC by 2 */
#ifdef DEBUG_TRACE
        fprintf(stderr, "SNE\n");
#endif

        if (vm->regs[x] != vm->regs[y])
            vm->PC += 2;
        break;
    }
    case 0xa:{
        /* 0xannn - LD I, nnn */
        /* Load addr (nnn) into I  */
#ifdef DEBUG_TRACE
        fprintf(stderr, "LD\n");
#endif

        vm->I = nnn;
        break;
    }
    case 0xb:{
        /* 0xbnnn - JP V0, nnn */
        /* Jump to V0 + nnn  */
#ifdef DEBUG_TRACE
        fprintf(stderr, "JP V0, %.3X\n", nnn);
#endif

        vm->PC = vm->regs[V0] + nnn;
        do_step = false;
        break;
    }
    case 0xc:{
        /* 0xcxkk - RND Vx, byte */
        /* Generate a random byte, AND with kk, store in Vx   */
#ifdef DEBUG_TRACE
        fprintf(stderr, "RND\n");
#endif

        vm->regs[x] = (rand() % 256) & kk;
        break;
    }
    case 0xd:{
        /* 0xdxyn - DRW Vx, Vy, nibble */
        /* Display n-byte sprite starting at memory I to location Vx, Vy, while
         * also setting VF to collision check result */
        bool is_pixel_erased = false;
        fb_draw_sprite(vm->display, &vm->ram[vm->I], n, vm->regs[x], vm->regs[y], &is_pixel_erased);
        vm->regs[Vf] = is_pixel_erased;

#ifdef DEBUG_TRACE
        fprintf(stderr, "DRW V%.1X, V%.1X, %u (%u %u %u)\n",
                x, y, n, vm->regs[x], vm->regs[y], vm->regs[Vf]);
#endif
        break;
    }
    case 0xe:{
        switch (kk) {
        case 0x9e:{
            /* 0xex9e - SKP Vx */
            /* Skip next instruction if key in Vx is currently pressed */
#ifdef DEBUG_TRACE
            fprintf(stderr, "SKP\n");
#endif

            bool is_pressed = false;
            key_evdev_is_key_pressed(vm->keyboard, vm->regs[x] , &is_pressed);
            if (is_pressed)
                vm->PC += 2;
            break;
        }
        case 0xa1:{
            /* 0xexa1 - SKNP Vx */
            /* Skep next instruction if key in Vx is currently NOT pressed */
#ifdef DEBUG_TRACE
            fprintf(stderr, "SKNP\n");
#endif

            bool is_pressed = false;
            key_evdev_is_key_pressed(vm->keyboard, vm->regs[x] , &is_pressed);
            if (!is_pressed)
                vm->PC += 2;
            break;
        }
        default:
            fprintf(stderr, "Unknown inner instruction: 0x%04x\n", instruction);
            exit(EXIT_FAILURE);
        }
        break;
    }
    case 0xf:{
        switch (kk) {
        case 0x07:{
            /* 0xfx07 - LD Vx, DT */
            /* Load DT into Vx */
#ifdef DEBUG_TRACE
            fprintf(stderr, "LD\n");
#endif

            vm->regs[x] = vm->DT;
            break;
        }
        case 0x0a:{
            /* 0xfx0a - LD Vx, K */
            /* Wait for a key press, store the value in Vx */

            int key_pressed = 0x0;
            int rc = key_evdev_wait_for_key(vm->keyboard, &key_pressed);
            vm->regs[x] = key_pressed;
#ifdef DEBUG_TRACE
            fprintf(stderr, "LD V%.1X, K\n", x);
            fprintf(stderr, "Pressed key %.1X (rc=%d)\n", key_pressed, rc);
#endif
            break;
        }
        case 0x15:{
            /* 0xfx15 - LD DT, Vx */
            /* Load Vx into DT */
#ifdef DEBUG_TRACE
            fprintf(stderr, "DT\n");
#endif

            vm->DT = vm->regs[x];
            break;
        }
        case 0x18:{
            /* 0xfx18 - LD ST, Vx */
            /* Load Vx into ST */
#ifdef DEBUG_TRACE
            fprintf(stderr, "LD\n");
#endif

            vm->ST = vm->regs[x];
            break;
        }
        case 0x1e:{
            /* 0xfx18 - ADD I, Vx */
            /* Load I + Vx into I */
#ifdef DEBUG_TRACE
            fprintf(stderr, "ADD\n");
#endif

            vm->I += vm->regs[x];
            break;
        }
        case 0x29:{
            /* 0xfx29 - LD F, Vx */
            /* Load location of digit Vx into I */
#ifdef DEBUG_TRACE
            fprintf(stderr, "LD\n");
#endif

            assert(vm->regs[x] < 16);

            vm->I = vm->regs[x] * 5;
            break;
        }
        case 0x33:{
            /* 0xfx18 - LD B, Vx */
            /* Load decimal hundreds, tens, ones of Vx into I, I+1, I+2 */
#ifdef DEBUG_TRACE
            fprintf(stderr, "LD\n");
#endif

            uint8_t reg_val = vm->regs[x];
            vm->ram[vm->I] = reg_val / 100;
            reg_val %= 100;
            vm->ram[vm->I + 1] = reg_val / 10;
            reg_val %= 10;
            vm->ram[vm->I + 2] = reg_val;
            break;
        }
        case 0x55:{
            /* 0xfx55 - LD [I], Vx */
            /* Dump registers V0 up to Vx into memory starting with addr I */
#ifdef DEBUG_TRACE
            fprintf(stderr, "LD\n");
#endif

            for (uint8_t i = 0; i <= x; ++i)
                vm->ram[vm->I + i] = vm->regs[i];
            break;
        }
        case 0x65:{
            /* 0xfx55 - LD Vx, [I] */
            /* Load registers V0 up to Vx from memory starting with addr I */
#ifdef DEBUG_TRACE
            fprintf(stderr, "LD\n");
#endif

            for (uint8_t i = 0; i <= x; ++i)
                vm->regs[i] = vm->ram[vm->I + i];
            break;
        }
        default:{
            fprintf(stderr, "Unknown inner instruction: 0x%04x\n", instruction);
            exit(EXIT_FAILURE);
        }

        }
        break;
    }
    default:{
        fprintf(stderr, "Unknown instruction: 0x%04x\n", instruction);
        exit(EXIT_FAILURE);
    }
    }

    if (do_step)
        vm->PC += 2;
}


void chip8_redraw(chip8 *vm)
{
    bool keyboard_state[CHIP8_KEY_COUNT] = {0};

    int rc = key_evdev_get_key_state(vm->keyboard, keyboard_state);
    if (rc != KEY_EVDEV_SUCCESS) {
        fprintf(stderr, "keyboard failure\n");
        exit(EXIT_FAILURE);
    }

    fb_redraw(vm->display, keyboard_state);
}

void chip8_cpu_tick(chip8 *vm)
{
    if (vm->usec_to_cpu_tick)
        return;

    /* TODO: check rc code */
    key_evdev_flush(vm->keyboard);

    uint16_t instruction = chip8_fetch(vm);
#ifdef DEBUG_TRACE
    fprintf(stderr, "PC: %.3X\n", vm->PC);
#endif
    chip8_exec(vm, instruction);

    vm->usec_to_cpu_tick += USECONDS_PER_STEP_CPU;
}

void chip8_timers_tick(chip8 *vm)
{
    /* some time left until the next tick? */
    if (vm->usec_to_timer_tick)
        return;

    if (vm->DT) {
        vm->DT -= 1;
    }

    if (vm->ST) {
        vm->ST -= 1;
    }

    vm->usec_to_timer_tick += USECONDS_PER_STEP_TIMER;
}

uint32_t chip8_tick(chip8 *vm)
{
    uint32_t usec_to_next = MIN(vm->usec_to_cpu_tick, vm->usec_to_timer_tick);
    vm->usec_to_cpu_tick -= usec_to_next;
    vm->usec_to_timer_tick -= usec_to_next;
    return usec_to_next;
}
