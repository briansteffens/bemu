#include <string.h>

#include "emulator.h"
#include "disassembler.h"

#define CLR_RESET   "\x1B[0m"
#define CLR_RED     "\x1B[31m"
#define CLR_GREEN   "\x1B[32m"
#define CLR_YELLOW  "\x1B[33m"
#define CLR_BLUE    "\x1B[34m"
#define CLR_MAGENTA "\x1B[35m"
#define CLR_CYAN    "\x1B[36m"
#define CLR_WHITE   "\x1B[37m"

void print_debug(machine_state* state)
{
    for (int i = 1; i < REGISTER_COUNT; i++)
    {
        if (i != RFLAG)
        {
            printf("\t%s: " CLR_BLUE "%llu" CLR_RESET,
               register_to_string(i),
               state->registers[i]);
        }
        else
        {
            printf("\t%s: " CLR_BLUE "%lli" CLR_RESET,
               register_to_string(i),
               state->registers[i]);
        }

        if ((i + 1) % 4 == 0 || i == REGISTER_COUNT - 1)
        {
            putchar('\n');
        }
    }

    putchar('\n');
}

void instruction_print(instruction* inst)
{
    printf(CLR_GREEN "%s", opcode_to_string(inst->opcode));

    if (inst->size != B8)
    {
        printf(" %s", size_to_string(inst->size));
    }

    int operand_count = operands(inst->opcode);
    char buffer[DEBUG_STR_LEN];

    for (int i = 0; i < operand_count; i++)
    {
        operand_to_string(inst, i, buffer);
        printf(" %s", buffer);
    }

    printf("\n" CLR_RESET);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: bdbg <binary_file>\n");
        return 1;
    }

    machine_state state;
    load_binary(argv[1], &state);

    do
    {
        print_debug(&state);

        instruction* inst;
        read_next_instruction(&state, &inst);

        printf(CLR_GREEN);
        instruction_print(inst);
        printf(CLR_RESET);

        // Debug prompt
        char* input = NULL;
        size_t len;
        int read = getline(&input, &len, stdin);

        if (read != -1)
        {
            // handle command
        }

        free(input);
    } while (execute(&state));

    print_debug(&state);

    free(state.memory);

    return 0;
}
