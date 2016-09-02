#include <stdlib.h>
#include <string.h>

#include "bstring.h"
#include "emulator.h"
#include "assembler.h"
#include "disassembler.h"

#define CLR_RESET   "\x1B[0m"
#define CLR_DIM     "\x1B[2m"
#define CLR_RED     "\x1B[31m"
#define CLR_GREEN   "\x1B[32m"
#define CLR_YELLOW  "\x1B[33m"
#define CLR_BLUE    "\x1B[34m"
#define CLR_MAGENTA "\x1B[35m"
#define CLR_CYAN    "\x1B[36m"
#define CLR_WHITE   "\x1B[37m"

#define MAX_PROMPT_LEN 255

uint64_t registers_last[REGISTER_COUNT];

bool will_jump(machine_state* state, instruction* inst)
{
    int64_t rflag = state->registers[RFLAG];

    switch (inst->opcode)
    {
        case OP_JE:  return rflag == 0;
        case OP_JNE: return rflag != 0;
        case OP_JL:  return rflag <  0;
        case OP_JLE: return rflag <= 0;
        case OP_JG:  return rflag >  0;
        case OP_JGE: return rflag >= 0;
        default:
            printf("Unrecognized opcode\n");
            exit(17);
    }
}

void print_debug(machine_state* state)
{
    for (int i = 0; i < REGISTER_COUNT; i++)
    {
        const bool changed = registers_last[i] != state->registers[i];

        printf("\t%s: ", register_to_string(i));
        printf(changed ? CLR_RED : CLR_BLUE);
        printf(i != RFLAG ? "%llu" : "%lli", state->registers[i]);
        printf(CLR_RESET);

        if ((i + 1) % 4 == 0 || i == REGISTER_COUNT - 1)
        {
            putchar('\n');
        }

        registers_last[i] = state->registers[i];
    }

    putchar('\n');
}

void instruction_print(machine_state* state, instruction* inst)
{
    printf(CLR_GREEN "%s", opcode_to_string(inst->opcode));

    if (inst->size != B8)
    {
        printf(" %s", size_to_string(inst->size));
    }

    int operand_count = operands[inst->opcode];
    char buffer[DEBUG_STR_LEN];

    for (int i = 0; i < operand_count; i++)
    {
        operand_to_string(inst, i, buffer);
        printf(" %s", buffer);
    }

    if (is_conditional_jump(inst->opcode) && will_jump(state, inst))
    {
        printf(CLR_RED " (will jump)");
    }

    printf("\n" CLR_RESET);
}

void instruction_bytecode_print(instruction* inst)
{
    unsigned char* raw = (unsigned char*)inst;

    int operand_count = operands[inst->opcode];

    printf(CLR_DIM);

    for (int i = 0; i < instruction_encoded_len(operand_count); i++)
    {
        printf("%.2x ", *(raw + i));

        if (i > 0 && (i + 1) % 8 == 0)
        {
            putchar(' ');
        }
    }

    printf(CLR_RESET "\n");
}

bool run_console_instruction(machine_state* state, bstring* line)
{
    bstring_trim(line);
    instruction inst;
    vec_bstring parts = parse_instruction_header(line, &inst);
    parse_instruction_operands(&parts, &inst);

    return execute_instruction(state, &inst);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: bdbg <binary_file>\n");
        return 1;
    }

    print_prefix = CLR_YELLOW;
    print_suffix = CLR_RESET;

    operands_init();

    machine_state state;
    load_binary(argv[1], &state);

    emulator_init();

    while (true)
    {
        print_debug(&state);

        instruction* inst;
        read_next_instruction(&state, &inst);

        instruction_bytecode_print(inst);

        printf(CLR_GREEN);
        instruction_print(&state, inst);
        printf(CLR_RESET);

        // Debug prompt
        printf("> ");

        char* input = NULL;
        size_t len;
        int read = getline(&input, &len, stdin);

        bool run_next_instruction = true;

        if (read != -1)
        {
            input[read - 1] = 0;

            if (input[0] == 'q')
            {
                break;
            }

            if (input[0] == '$')
            {
                bstring binput = bstring_from_char(input + 1);

                if (!run_console_instruction(&state, &binput))
                {
                    break;
                }

                continue;
            }
        }

        free(input);

        if (run_next_instruction)
        {
            if (!execute(&state))
            {
                break;
            }
        }
    }

    print_debug(&state);

    free(state.memory);

    return 0;
}
