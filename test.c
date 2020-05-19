#include <assert.h>
#include <string.h>
#include "chip8.h"

int main(int argc, char *argv[])
{
#define INSTR_NNN(type, arg)                    \
    (((0x000f & type) << 12) | (0x0fff & arg))

#define INSTR_XKK(type, x, kk)                                          \
    (((0x000f & type) << 12) | ((0x000f & x) << 8) | (0x00ff & kk))
#define INSTR_XY(type, x, y)                                            \
    (((0x000f & type) << 12) | ((0x000f & x) << 8) | ((0x00ff & y) << 4))

    (void) argc; (void) argv;

    /* TODO 30 left*/
    {
        /* SYS */

        chip8 vm1, vm2;
        chip8_reset(&vm1);
        chip8_reset(&vm2);

        chip8_exec(&vm1, INSTR_NNN(0x0, 0x0111));

        /* should be equal */
        assert(0 == memcmp(&vm1, &vm2, sizeof(chip8)));
    }

    {
        /* CLS */
        /* TODO: */
    }

    {
        /* RET */

        chip8 vm;
        chip8_reset(&vm);

        vm.SP = 1;
        vm.stack[0] = 0x0222;
        chip8_exec(&vm, INSTR_NNN(0x0, 0x0ee));

        assert(vm.PC == 0x0222);
        assert(vm.SP == 0);
    }

    {
        /* JP */

        chip8 vm;
        chip8_reset(&vm);

        assert(vm.PC == PROGRAM_START);

        chip8_exec(&vm, INSTR_NNN(0x1, 0x0111));

        assert(vm.PC == 0x0111);
    }

    {
        /* CALL */

        chip8 vm;
        chip8_reset(&vm);

        assert(vm.stack[0] == 0);
        assert(vm.PC == PROGRAM_START);
        assert(vm.SP == 0);

        chip8_exec(&vm, INSTR_NNN(0x2, 0x0111));

        assert(vm.stack[0] == PROGRAM_START);
        assert(vm.PC == 0x0111);
        assert(vm.SP == 1);
    }

    {
        /* SE Vx, byte */

        chip8 vm;
        chip8_reset(&vm);

        vm.regs[V1] = 0x11;

        chip8_exec(&vm, INSTR_XKK(0x3, V1, 0x22));
        assert(vm.PC == PROGRAM_START);

        chip8_exec(&vm, INSTR_XKK(0x3, V1, 0x11));
        assert(vm.PC == PROGRAM_START + 2);
    }

    {
        /* SNE Vx, byte*/

        chip8 vm;
        chip8_reset(&vm);

        vm.regs[V1] = 0x11;

        chip8_exec(&vm, INSTR_XKK(0x4, V1, 0x11));
        assert(vm.PC == PROGRAM_START);

        chip8_exec(&vm, INSTR_XKK(0x4, V1, 0x22));
        assert(vm.PC == PROGRAM_START + 2);
    }

    {
        /* SE Vx, Vy*/

        chip8 vm;
        chip8_reset(&vm);

        vm.regs[V1] = 0x11;
        vm.regs[V2] = 0x11;
        vm.regs[V3] = 0x22;

        chip8_exec(&vm, INSTR_XY(0x5, V1, V3));
        assert(vm.PC == PROGRAM_START);

        chip8_exec(&vm, INSTR_XY(0x5, V1, V2));
        assert(vm.PC == PROGRAM_START + 2);
    }

    return 0;

#undef INSTR_NNN
#undef INSTR_XKK
#undef INSTR_XY
}
