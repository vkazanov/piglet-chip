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
    case 0:{

    }
    default:{
        fprintf(stderr, "Unknown instruction: %04x\n", instruction);
        exit(EXIT_FAILURE);
    }
    }
}
