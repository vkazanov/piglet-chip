#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>


void chip8_reset(chip8 *vm)
{
    *vm = (chip8){0};
    vm->PC = PROGRAM_START;
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

    /* TODO: The super switch */
    switch (type) {
    case 0x0:{
        switch (nnn) {
        case 0x0e0:{
            /* 00e0 - CLS */
            /* Clear screen */

            /* TODO */
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
        /* 0x8xy0 - LD Vx, Vy */
        /* Load Vy into Vx */

        vm->regs[x] = vm->regs[y];
        break;
    }
    default:{
        fprintf(stderr, "Unknown instruction: 0x%04x\n", instruction);
        exit(EXIT_FAILURE);
    }
    }
}
