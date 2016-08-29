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

byte* resolve_operand(machine_state* state, operand* oper)
{
    byte* ret = NULL;

    switch (oper->type)
    {
        case IMMEDIATE:
            ret = (byte*)&oper->data;
            break;

        case REGISTER:
            ret = (byte*)&state->registers[operand_unpack_register(oper)];
            break;

        default:
            printf("Unrecognized operand type\n");
            exit(3);
    }

    switch (oper->mode)
    {
        case LITERAL:
            break;

        case ADDRESS:
            ret = &state->memory[*(u64*)ret + operand_unpack_offset(oper)];
            break;

        default:
            printf("Unrecognized operand mode\n");
            exit(4);
    }

    return ret;
}

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
            state->registers[RIP] = *(u64*)resolve_operand(
                    state,
                    &inst.operands[0]);

            break;

        case OP_PUSH:
            state->registers[RSP] -= inst.size;

            memcpy(state->memory + state->registers[RSP],
                   resolve_operand(state, &inst.operands[0]),
                   inst.size);

            break;

        case OP_POP:
            memcpy(resolve_operand(state, &inst.operands[0]),
                   state->memory + state->registers[RSP],
                   inst.size);

            state->registers[RSP] += inst.size;

            break;

        case OP_MOV:
            memcpy(resolve_operand(state, &inst.operands[0]),
                   resolve_operand(state, &inst.operands[1]),
                   inst.size);

        case OP_EXIT:
            return false;

        default:
            printf("Unrecognized opcode\n");
            exit(3);
    }

    return true;
}

void print_debug(machine_state* state)
{
    printf("  R0: %llu\n", state->registers[R0]);
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

    // Clear registers
    for (int i = 0; i < REGISTER_COUNT; i++)
    {
        state.registers[i] = 0;
    }

    state.registers[RIP] = *(u64 *)(bytes + IMG_HDR_ENTRY_POINT);
    state.registers[RSP] = MEMORY_SIZE;

    state.memory = malloc(sizeof(byte) * MEMORY_SIZE);
    memcpy(state.memory, bytes + IMG_HDR_LEN, code_bytes);

    // Clear non-code memory
    for (int i = code_bytes; i < MEMORY_SIZE; i++)
    {
        state.memory[i] = 0;
    }

    free(bytes);

    while (execute(&state))
    {
        print_debug(&state);
    }

    print_debug(&state);

    free(state.memory);

    return 0;
}
