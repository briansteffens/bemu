#include <string.h>

#include "emulator.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: bemu <binary_file>\n");
        return 1;
    }

    operands_init();

    machine_state state;
    load_binary(argv[1], &state);

    emulator_init();

    while (execute(&state)) { }

    free(state.memory);

    return 0;
}
