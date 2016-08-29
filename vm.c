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

void push(machine_state* state, byte* data, byte size)
{
    state->registers[RSP] -= size;

    memcpy(state->memory + state->registers[RSP], data, size);
}

void pop(machine_state* state, byte* target, byte size)
{
    memcpy(target, state->memory + state->registers[RSP], size);

    state->registers[RSP] += size;
}

void execute_jmp(machine_state* state, instruction* inst)
{
    state->registers[RIP] = *(u64*)resolve_operand(state, &inst->operands[0]);
}

void execute_push(machine_state* state, instruction* inst)
{
    push(state, resolve_operand(state, &inst->operands[0]), inst->size);
}

void execute_pop(machine_state* state, instruction* inst)
{
    pop(state, resolve_operand(state, &inst->operands[0]), inst->size);
}

void execute_mov(machine_state* state, instruction* inst)
{
    memcpy(resolve_operand(state, &inst->operands[0]),
           resolve_operand(state, &inst->operands[1]),
           inst->size);
}

void execute_call(machine_state* state, instruction* inst)
{
    push(state, (byte*)&state->registers[RIP], B8);
    execute_jmp(state, inst);
}

void execute_ret(machine_state* state, instruction* inst)
{
    pop(state, (byte*)&state->registers[RIP], B8);
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
            execute_jmp(state, &inst);
            break;

        case OP_PUSH:
            execute_push(state, &inst);
            break;

        case OP_POP:
            execute_pop(state, &inst);
            break;

        case OP_MOV:
            execute_mov(state, &inst);
            break;

        case OP_CALL:
            execute_call(state, &inst);
            break;

        case OP_RET:
            execute_ret(state, &inst);
            break;

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
    printf("  R0: %llu", state->registers[R0]);
    printf("  R1: %llu", state->registers[R1]);
    putchar('\n');
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
