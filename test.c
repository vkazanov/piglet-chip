#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "chip8.h"
#include "key-evdev.h"

int main(int argc, char *argv[])
{
#define INSTR_NNN(type, arg)                    \
    (((0x000f & type) << 12) | (0x0fff & arg))

#define INSTR_XKK(type, x, kk)                                          \
    (((0x000f & type) << 12) | ((0x000f & x) << 8) | (0x00ff & kk))
#define INSTR_XY(type, x, y)                                            \
    (((0x000f & type) << 12) | ((0x000f & x) << 8) | ((0x00ff & y) << 4))
#define INSTR_XY_N(type, x, y, n)                                       \
    (((0x000f & type) << 12) | ((0x000f & x) << 8) | ((0x00ff & y) << 4) | \
     (0x000f & n))
    (void) argc; (void) argv;

    setbuf(stdout, NULL);

    key_evdev *keyboard = NULL;
    /* TODO: keyboard device files do change on reboots  */
    int rc = key_evdev_new("/dev/input/event7", &keyboard);
    assert(KEY_EVDEV_SUCCESS == rc);

    /* TODO: err code checks */
    fb_console *display = NULL;
    fb_new(&display);

    {
        /* SYS */

        chip8 vm1, vm2;
        chip8_reset(&vm1, keyboard, display);
        chip8_reset(&vm2, keyboard, display);

        chip8_exec(&vm1, INSTR_NNN(0x0, 0x0111));

        /* should be equal */
        assert(0 == memcmp(&vm1, &vm2, sizeof(chip8)));
    }

    {
        /* CLS */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        printf("clear screen in 1s...\n");
        sleep(1);
        chip8_exec(&vm, INSTR_NNN(0x0, 0x00e0));
        chip8_maybe_redraw(&vm);
    }

    {
        /* RET */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.SP = 1;
        vm.stack[0] = 0x0222;
        chip8_exec(&vm, INSTR_NNN(0x0, 0x0ee));

        assert(vm.PC == 0x0222);
        assert(vm.SP == 0);
    }

    {
        /* JP */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        assert(vm.PC == PROGRAM_START);

        chip8_exec(&vm, INSTR_NNN(0x1, 0x0111));

        assert(vm.PC == 0x0111);
    }

    {
        /* CALL */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

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
        chip8_reset(&vm, keyboard, display);

        vm.regs[V1] = 0x11;

        chip8_exec(&vm, INSTR_XKK(0x3, V1, 0x22));
        assert(vm.PC == PROGRAM_START);

        chip8_exec(&vm, INSTR_XKK(0x3, V1, 0x11));
        assert(vm.PC == PROGRAM_START + 2);
    }

    {
        /* SNE Vx, byte*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V1] = 0x11;

        chip8_exec(&vm, INSTR_XKK(0x4, V1, 0x11));
        assert(vm.PC == PROGRAM_START);

        chip8_exec(&vm, INSTR_XKK(0x4, V1, 0x22));
        assert(vm.PC == PROGRAM_START + 2);
    }

    {
        /* SE Vx, Vy*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V1] = 0x11;
        vm.regs[V2] = 0x11;
        vm.regs[V3] = 0x22;

        chip8_exec(&vm, INSTR_XY(0x5, V1, V3));
        assert(vm.PC == PROGRAM_START);

        chip8_exec(&vm, INSTR_XY(0x5, V1, V2));
        assert(vm.PC == PROGRAM_START + 2);
    }

    {
        /* LD Vx, byte*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        chip8_exec(&vm, INSTR_XKK(0x6, V1, 10));
        assert(vm.regs[V1] == 10);

        chip8_exec(&vm, INSTR_XKK(0x6, V1, 12));
        assert(vm.regs[V1] == 12);
    }

    {
        /* ADD Vx, byte*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        chip8_exec(&vm, INSTR_XKK(0x7, V1, 10));
        assert(vm.regs[V1] == 10);
        chip8_exec(&vm, INSTR_XKK(0x7, V1, 12));
        assert(vm.regs[V1] == 22);
    }

    {
        /* LD Vx, Vy*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V2] = 0x22;
        vm.regs[V3] = 0x33;

        chip8_exec(&vm, INSTR_XY_N(0x8, V1, V2, 0x0));
        assert(vm.regs[V1] == 0x22);

        chip8_exec(&vm, INSTR_XY_N(0x8, V1, V3, 0x0));
        assert(vm.regs[V1] == 0x33);
    }

    {
        /* OR Vx, Vy*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V2] = 0x1;
        vm.regs[V3] = 0x2;

        chip8_exec(&vm, INSTR_XY_N(0x8, V1, V2, 0x1));
        assert(vm.regs[V1] == 0x1);

        chip8_exec(&vm, INSTR_XY_N(0x8, V2, V3, 0x1));
        assert(vm.regs[V2] == 0x3);
    }

    {
        /* AND Vx, Vy*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V2] = 0x1;
        vm.regs[V3] = 0x3;

        chip8_exec(&vm, INSTR_XY_N(0x8, V1, V2, 0x2));
        assert(vm.regs[V1] == 0x0);

        chip8_exec(&vm, INSTR_XY_N(0x8, V2, V3, 0x2));
        assert(vm.regs[V2] == 0x1);
    }

    {
        /* XOR Vx, Vy*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V2] = 0x1;
        vm.regs[V3] = 0x1;

        chip8_exec(&vm, INSTR_XY_N(0x8, V1, V2, 0x3));
        assert(vm.regs[V1] == 0x1);

        chip8_exec(&vm, INSTR_XY_N(0x8, V2, V3, 0x3));
        assert(vm.regs[V2] == 0x0);
    }

    {
        /* ADD Vx, Vy*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V1] = 0x1;
        vm.regs[V2] = 0x2;
        vm.regs[V3] = 0x2;
        vm.regs[V4] = 0xff;

        chip8_exec(&vm, INSTR_XY_N(0x8, V1, V2, 0x4));
        assert(vm.regs[V1] == 0x3);
        assert(!vm.regs[Vf]);

        chip8_exec(&vm, INSTR_XY_N(0x8, V3, V4, 0x4));
        assert(vm.regs[V3] == 0x1);
        assert(vm.regs[Vf]);
    }

    {
        /* SUB Vx, Vy*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V1] = 0x1;
        vm.regs[V2] = 0x2;
        vm.regs[V3] = 0x3;

        chip8_exec(&vm, INSTR_XY_N(0x8, V3, V1, 0x5));
        assert(vm.regs[V3] == 0x2);
        assert(vm.regs[Vf]);

        chip8_exec(&vm, INSTR_XY_N(0x8, V3, V2, 0x5));
        assert(vm.regs[V3] == 0x0);
        assert(!vm.regs[Vf]);
    }

    {
        /* SHR Vx, Vy*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V1] = 0x3;
        vm.regs[V2] = 0x2;

        chip8_exec(&vm, INSTR_XY_N(0x8, V1, V0, 0x6));
        assert(vm.regs[V1] == 0x1);
        assert(vm.regs[Vf] == 1);

        chip8_exec(&vm, INSTR_XY_N(0x8, V2, V0, 0x6));
        assert(vm.regs[V2] == 0x1);
        assert(vm.regs[Vf] == 0x0);
    }

    {
        /* SUBN Vx, Vy*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V1] = 0x1;
        vm.regs[V2] = 0x3;
        vm.regs[V3] = 0x3;

        chip8_exec(&vm, INSTR_XY_N(0x8, V1, V3, 0x7));
        assert(vm.regs[V1] == 0x2);
        assert(vm.regs[Vf]);

        chip8_exec(&vm, INSTR_XY_N(0x8, V3, V2, 0x7));
        assert(vm.regs[V3] == 0x0);
        assert(!vm.regs[Vf]);
    }

    {
        /* SHL Vx, Vy*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V1] = 0x1;
        vm.regs[V2] = (0x1 << 7) | 0x1;

        chip8_exec(&vm, INSTR_XY_N(0x8, V1, V0, 0x8));
        assert(vm.regs[V1] == 0x2);
        assert(vm.regs[Vf] == 0);

        chip8_exec(&vm, INSTR_XY_N(0x8, V2, V0, 0x8));
        assert(vm.regs[V2] == 0x2);
        assert(vm.regs[Vf] == 1);
    }

    {
        /* SNE Vx, Vy */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V1] = 0x1;
        vm.regs[V2] = 0x1;
        vm.regs[V3] = 0x2;

        uint16_t orig_PC = vm.PC;

        chip8_exec(&vm, INSTR_XY(0x9, V1, V2));
        assert(orig_PC == vm.PC);

        chip8_exec(&vm, INSTR_XY(0x9, V1, V3));
        assert(orig_PC + 2 == vm.PC);
    }

    {
        /* LD I, addr */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.I = 0x1;
        assert(vm.I == 0x1);
        chip8_exec(&vm, INSTR_NNN(0xa, 0x2));
        assert(vm.I == 0x2);
    }

    {
        /* JP V0, addr */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V0] = 0x1;
        chip8_exec(&vm, INSTR_NNN(0xb, 0x2));
        assert(vm.PC == 0x3);
    }

    {
        /* RND Vx, byte */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.regs[V0] = 0x1;
        chip8_exec(&vm, INSTR_XKK(0xc, V0, 0x00));
        assert(vm.regs[V0] == 0x0);

        /* NOTE: random generators are hard to test  */
    }

    {
        /* TODO: DRW Vx, Vy, nibble */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);
        size_t addr = (1 << 11); /* addr 2048 */
        vm.I = addr;

        vm.ram[addr] = 0x0f;
        vm.ram[addr + 1] = 0xff;
        vm.ram[addr + 2] = 0xff;
        vm.ram[addr + 3] = 0xf0;

        vm.regs[V0] = 0x1;      /* x */
        vm.regs[V1] = 0x2;      /* y */

        sleep(1);
        chip8_exec(&vm, INSTR_XY_N(0xd, V0, V1, 0x4));
        chip8_maybe_redraw(&vm);
        printf("custom sprite...");
        sleep(1);

        vm.I = 0x0;             /* num addr */
        vm.regs[V0] = 32;      /* x */
        vm.regs[V1] = 16;      /* y */
        chip8_exec(&vm, INSTR_NNN(0x0, 0x00e0)); /* CLS */
        chip8_exec(&vm, INSTR_XY_N(0xd, V0, V1, 0x5));
        chip8_maybe_redraw(&vm);

        printf("preloaded sprites...");
        sleep(1);

        for (size_t i = 0; i < 16; i++ ) {
            vm.I = i * 5;
            chip8_exec(&vm, INSTR_NNN(0x0, 0x00e0));
            chip8_exec(&vm, INSTR_XY_N(0xd, V0, V1, 0x5));
            chip8_maybe_redraw(&vm);
            sleep(1);
        }
    }

    return 0;

    {
        /* SKP Vx  */
            chip8 vm;
            chip8_reset(&vm, keyboard, display);

            printf("keep pressing 1 to make this test pass...\n");
            sleep(2);

            vm.PC = 0x1;
            vm.regs[V1] = 0x1;
            chip8_exec(&vm, INSTR_XKK(0xe, V1, 0x9e));
            assert(vm.PC == 0x3);
        }

    /* SKNP Vx  */
    {
        /* SKP Vx  */
        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        printf("do NOT press 1 to make this test pass...\n");
        sleep(2);

        vm.PC = 0x1;
        vm.regs[V1] = 0x1;
        chip8_exec(&vm, INSTR_XKK(0xe, V1, 0xa1));
        assert(vm.PC == 0x3);
    }

    {
        /* LD Vx, DT */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.DT = 0x11;
        vm.regs[V1] = 0x10;
        chip8_exec(&vm, INSTR_XKK(0xf, V1, 0x07));
        assert(vm.regs[V1] == 0x11);
    }

    {
        /* LD Vx, K */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        printf("press 2 to make this test pass...\n");

        chip8_exec(&vm, INSTR_XKK(0xf, V2, 0x0a));
        assert(vm.regs[V2] == 0x2);
    }

    {
        /* LD DT, Vx */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.DT = 0x11;
        vm.regs[V1] = 0x10;
        chip8_exec(&vm, INSTR_XKK(0xf, V1, 0x15));
        assert(vm.DT == 0x10);
    }

    {
        /* LD ST, Vx */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.ST = 0x11;
        vm.regs[V1] = 0x10;
        chip8_exec(&vm, INSTR_XKK(0xf, V1, 0x18));
        assert(vm.ST == 0x10);
    }

    {
        /* ADD I, Vx */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.I = 0x11;
        vm.regs[V1] = 0x2;
        chip8_exec(&vm, INSTR_XKK(0xf, V1, 0x1E));
        assert(vm.I == 0x13);
    }

    {
        /* LD F, Vx */
        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.I = 0x11;
        vm.regs[V1] = 0x2;
        chip8_exec(&vm, INSTR_XKK(0xf, V1, 0x29));
        assert(vm.I == 5 * 2);
    }

    return 0;

    {
        /* LD B, Vx*/

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.I = 0x2;
        vm.regs[V1] = 123;
        chip8_exec(&vm, INSTR_XKK(0xf, V1, 0x33));
        assert(vm.ram[vm.I] == 0x1);
        assert(vm.ram[vm.I + 1] == 0x2);
        assert(vm.ram[vm.I + 2] == 0x3);
    }

    {
        /* LD [I], Vx */

        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.I = 0x2;
        vm.regs[V1] = 1;
        vm.regs[V2] = 2;
        vm.regs[V3] = 3;

        chip8_exec(&vm, INSTR_XKK(0xf, V3, 0x55));

        assert(vm.ram[vm.I] == 0);
        assert(vm.ram[vm.I + 1] == 1);
        assert(vm.ram[vm.I + 2] == 2);
        assert(vm.ram[vm.I + 3] == 3);
    }

    {
        /* LD Vx, [I] */
        chip8 vm;
        chip8_reset(&vm, keyboard, display);

        vm.I = 0x2;
        vm.ram[vm.I] = 0;
        vm.ram[vm.I + 1] = 1;
        vm.ram[vm.I + 2] = 2;
        vm.ram[vm.I + 3] = 3;

        chip8_exec(&vm, INSTR_XKK(0xf, V3, 0x65));

        assert(vm.regs[V0] == 0);
        assert(vm.regs[V1] == 1);
        assert(vm.regs[V2] == 2);
        assert(vm.regs[V3] == 3);


    }

    fb_free(display);
    key_evdev_free(keyboard);

    return 0;

#undef INSTR_NNN
#undef INSTR_XKK
#undef INSTR_XY
#undef INSTR_XY_N
}
