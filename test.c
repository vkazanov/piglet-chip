#include <assert.h>
#include <string.h>
#include "chip8.h"

int main(int argc, char *argv[])
{
#define INSTR_NNN(type, arg)                    \
    (((0x000f & type) << 12) | (0x0fff & arg))

    (void) argc; (void) argv;

    /* TODO 35 left*/
    {
        /* SYS */

        chip8 vm1, vm2;
        chip8_reset(&vm1);
        chip8_reset(&vm2);

        chip8_exec(&vm1, INSTR_NNN(0, 0x0111));

        /* should be equal */
        assert(memcmp(&vm1, &vm2, sizeof(chip8)));
    }

    {
        /* JP */
        /* TODO: */
    }

    {
        /* CALL */
        /* TODO: */

    }

    return 0;

#undef INSTR_NNN
}
