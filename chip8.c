#include "chip8.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static void load_sprites(chip8 *vm);

void chip8_reset(chip8 *vm, key_evdev *keyboard, fb_console *display)
{
    *vm = (chip8){0};
    vm->PC = PROGRAM_START;
    vm->keyboard = keyboard;
    vm->display = display;
    assert(KEY_EVDEV_SUCCESS == key_evdev_flush(vm->keyboard));

    load_sprites(vm);
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

void chip8_bump_PC(chip8 *vm)
{
    vm->PC += 2;
}

void chip8_exec(chip8 *vm, uint16_t instruction)
{
    uint16_t type = (0xF000 & instruction) >> 12;
    uint16_t x = (0x0F00 & instruction) >> 8;
    uint16_t y = (0x00F0 & instruction) >> 4;
    uint16_t nnn = (0x0FFF & instruction);
    uint16_t kk = (0x00FF & instruction);
    uint16_t n = (0x000F & instruction);

    switch (type) {
    case 0x0:{
        switch (nnn) {
        case 0x0e0:{
            /* 00e0 - CLS */
            /* Clear screen */

            fb_clear(vm->display);
            break;
        }
        case 0x0ee:{
            /* 00e0 - RET */
            /* Return from a subroutine */

            vm->SP--;
            vm->PC = vm->stack[vm->SP];
            break;
        }
        default:{
            /* 0nnn - SYS addr */
            /* Jump to a routine at nnn (currently ignored) */

            break;
        }
        }
        break;
    }
    case 0x1:{
        /* 0x1nnn - JP addr */
        /* Jump to addr */

        vm->PC = nnn;
        break;
    }
    case 0x2:{
        /* 0x2nnn - CALL addr */
        /* A subroutine call at addr */

        vm->stack[vm->SP] = vm->PC;
        vm->SP++;
        vm->PC = nnn;
        break;
    }
    case 0x3:{
        /* 0x3xkk - SE Vx, byte */
        /* Compare value in Vx with byte kk, skip instr if equal */

        if (vm->regs[x] == kk) {
            vm->PC += 2;
        }
        break;
    }
    case 0x4:{
        /* 0x4xkk - SNE Vx, byte */
        /* Compare value in Vx with byte kk, skip instr if NOT equal */

        if (vm->regs[x] != kk) {
            vm->PC += 2;
        }
        break;
    }
    case 0x5:{
        /* 0x5xy0 - SE Vx, Vy */
        /* Compare value in Vx with value in Vy, skip instr if equal */

        if (vm->regs[x] == vm->regs[y]) {
            vm->PC += 2;
        }
        break;
    }
    case 0x6:{
        /* 0x6xkk - LD Vx, byte */
        /* Load kk into Vx */

        vm->regs[x] = kk;
        break;
    }
    case 0x7:{
        /* 0x7xkk - ADD Vx, byte */
        /* Add kk to the value in Vx */

        vm->regs[x] += kk;
        break;
    }
    case 0x8:{
        switch (n) {
        case 0x0:{
            /* 0x8xy0 - LD Vx, Vy */
            /* Load Vy into Vx */

            vm->regs[x] = vm->regs[y];
            break;
        }
        case 0x1:{
            /* 0x8xy1 - OR Vx, Vy */
            /* OR Vy into Vx */

            vm->regs[x] |= vm->regs[y];
            break;
        }
        case 0x2:{
            /* 0x8xy2 - AND Vx, Vy */
            /* AND Vy into Vx */

            vm->regs[x] &= vm->regs[y];
            break;
        }
        case 0x3:{
            /* 0x8xy3 - XOR Vx, Vy */
            /* XOR Vy into Vx */

            vm->regs[x] ^= vm->regs[y];
            break;
        }
        case 0x4:{
            /* 0x8xy4 - ADD Vx, Vy */
            /* ADD Vy into Vx, with carry to VF */

            uint16_t acc = vm->regs[x] + vm->regs[y];
            vm->regs[x] = acc & 0xff;
            vm->regs[Vf] = (acc & 0xf00) >> 8;
            break;
        }
        case 0x5:{
            /* 0x8xy5 - SUB Vx, Vy */
            /* SUB Vy from Vx, with borrow result to VF */

            /* NOTE: is it > or >=? Is the reference wrong? */
            vm->regs[Vf] = (vm->regs[x] > vm->regs[y]) ? 1 : 0;
            vm->regs[x] -= vm->regs[y];
            break;
        }
        case 0x6:{
            /* 0x8xy6 - SHR Vx */
            /* SHR shift Vx right, if the shifted bit was 1 - set VF to 1,
             * otherwise - to 0 */

            vm->regs[Vf] = vm->regs[x] & 0x1;
            vm->regs[x] >>= 1;
            break;
        }
        case 0x7:{
            /* 0x8xy7 - SUBN Vx, Vy */
            /* SUB Vx from Vy, with borrow result to VF,
             * otherwise - to 0 */

            vm->regs[Vf] = (vm->regs[y] > vm->regs[x]) ? 1 : 0;
            vm->regs[x] = vm->regs[y] - vm->regs[x];
            break;
        }
        case 0x8:{
            /* 0x8xy8 - SHL Vx */
            /* SHR shift Vx left, if the shifted bit was 1 - set VF to 1,
             * otherwise - to 0 */

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
        if (vm->regs[x] != vm->regs[y])
            vm->PC += 2;
        break;
    }
    case 0xa:{
        /* 0xannn - LD I, nnn */
        /* Load addr (nnn) into I  */
        vm->I = nnn;
        break;
    }
    case 0xb:{
        /* 0xbnnn - JP V0, nnn */
        /* Jump to V0 + nnn  */
        vm->PC = vm->regs[V0] + nnn;
        break;
    }
    case 0xc:{
        /* 0xcxkk - RND Vx, byte */
        /* Generate a random byte, AND with kk, store in Vx   */

        vm->regs[x] = (rand() % 256) & kk;
        break;
    }
    case 0xd:{
        /* 0xdxyn - DRW Vx, Vy, nibble */
        /* Display n-byte sprite starting at memory I to location Vx, Vy, while
         * also setting VF to collision check result */

        bool is_pixel_erased = false;
        fb_draw_sprite(vm->display, &vm->ram[vm->I], n, vm->regs[x], vm->regs[y], &is_pixel_erased);
        if (is_pixel_erased)
            vm->regs[Vf] = is_pixel_erased;

        break;
    }
    case 0xe:{
        switch (kk) {
        case 0x9e:{
            /* 0xex9e - SKP Vx */
            /* Skip next instruction if key in Vx is currently pressed */
            bool is_pressed = false;
            key_evdev_is_key_pressed(vm->keyboard, vm->regs[x] , &is_pressed);
            if (is_pressed)
                vm->PC += 2;
            printf(is_pressed ? "pressed\n" : "not pressed\n");
            break;
        }
        case 0xa1:{
            /* 0xexa1 - SKNP Vx */
            /* Skep next instruction if key in Vx is currently NOT pressed */
            bool is_pressed = false;
            key_evdev_is_key_pressed(vm->keyboard, vm->regs[x] , &is_pressed);
            if (!is_pressed)
                vm->PC += 2;
            printf(is_pressed ? "pressed\n" : "not pressed\n");
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

            vm->regs[x] = vm->DT;
            break;
        }
        case 0x0a:{
            /* 0xfx0a - LD Vx, K */
            /* Wait for a key press, store the value in Vx */

            int key_pressed = 0x0;
            key_evdev_wait_for_key(vm->keyboard, &key_pressed);
            vm->regs[x] = key_pressed;
            break;
        }
        case 0x15:{
            /* 0xfx15 - LD DT, Vx */
            /* Load Vx into DT */

            vm->DT = vm->regs[x];
            break;
        }
        case 0x18:{
            /* 0xfx18 - LD ST, Vx */
            /* Load Vx into ST */

            vm->ST = vm->regs[x];
            break;
        }
        case 0x1e:{
            /* 0xfx18 - ADD I, Vx */
            /* Load I + Vx into I */

            vm->I += vm->regs[x];
            break;
        }
        case 0x29:{
            /* 0xfx29 - LD F, Vx */
            /* Load location of digit Vx into I */

            assert(vm->regs[x] < 16);

            vm->I = vm->regs[x] * 5;
            break;
        }
        case 0x33:{
            /* 0xfx18 - LD B, Vx */
            /* Load decimal hundreds, tens, ones of Vx into I, I+1, I+2 */

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

            for (uint8_t i = 0; i <= x; ++i)
                vm->ram[vm->I + i] = vm->regs[i];
            break;
        }
        case 0x65:{
            /* 0xfx55 - LD Vx, [I] */
            /* Load registers V0 up to Vx from memory starting with addr I */

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
}


void chip8_maybe_redraw(chip8 *vm)
{
    fb_redraw(vm->display);
}
