#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "shared.h"

#define MEMORY_SIZE 32 * 1024 * 1024

typedef struct
{
    u64 registers[REGISTER_COUNT];
    byte* memory;
} machine_state;

bool execute(machine_state* state)
{
    instruction inst;

    state->registers[RIP] += instruction_decode(
            state->memory + state->registers[RIP],
            &inst);

    instruction_print(&inst);

    switch (inst.opcode)
    {
        case OP_JMP:
            state->registers[RIP] = inst.operands[0].data;
            break;

        case OP_PUSH:
            state->registers[RSP] -= 8;
            encode_u64(
                    inst.operands[0].data,
                    state->memory + state->registers[RSP]);
            break;

        case OP_POP:
            state->registers[inst.operands[0].data] =
                    *(u64*)(state->memory + state->registers[RSP]);
            state->registers[RSP] += 8;
            break;

        case OP_EXIT:
            return false;

        default:
            printf("Unrecognized opcode\n");
            exit(3);
    }

    return true;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: bvm <binary_file>\n");
        return 1;
    }

    int bytes_count;
    byte* bytes = read_file(argv[1], &bytes_count);

    u64 code_bytes = *(u64 *)(bytes + IMG_HDR_CODE_BYTES);

    machine_state state;

    state.registers[RIP] = *(u64 *)(bytes + IMG_HDR_ENTRY_POINT);
    state.registers[RSP] = MEMORY_SIZE;

    state.memory = malloc(sizeof(byte) * MEMORY_SIZE);
    memcpy(state.memory, bytes + IMG_HDR_LEN, code_bytes);

    free(bytes);

    while (execute(&state))
    {
        printf("  R0: %llu\n", state.registers[R0]);
    }

    free(state.memory);

    return 0;
}
