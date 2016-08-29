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

void jump(machine_state* state, instruction* inst)
{
    state->registers[RIP] = *(u64*)resolve_operand(state, &inst->operands[0]);
}

void execute_jmp(machine_state* state, instruction* inst)
{
    jump(state, inst);
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

void basic_math(
        machine_state* state,
        instruction* inst,
        u64 (*handler)(u64, u64))
{
    byte* target = resolve_operand(state, &inst->operands[0]);
    u64 left = *(u64*)target;
    u64 right = *(u64*)resolve_operand(state, &inst->operands[1]);
    u64 result = handler(left, right);
    memcpy(target, &result, inst->size);
}

u64 math_handler_add(u64 left, u64 right) { return left + right; }
u64 math_handler_sub(u64 left, u64 right) { return left - right; }
u64 math_handler_mul(u64 left, u64 right) { return left * right; }
u64 math_handler_div(u64 left, u64 right) { return left / right; }
u64 math_handler_mod(u64 left, u64 right) { return left % right; }

void execute_add(machine_state* state, instruction* inst)
{
    basic_math(state, inst, math_handler_add);
}

void execute_sub(machine_state* state, instruction* inst)
{
    basic_math(state, inst, math_handler_sub);
}

void execute_mul(machine_state* state, instruction* inst)
{
    basic_math(state, inst, math_handler_mul);
}

void execute_div(machine_state* state, instruction* inst)
{
    basic_math(state, inst, math_handler_div);
}

void execute_mod(machine_state* state, instruction* inst)
{
    basic_math(state, inst, math_handler_mod);
}

void execute_inc(machine_state* state, instruction* inst)
{
    byte* target = resolve_operand(state, &inst->operands[0]);
    u64 result = ++*(u64*)target;
    memcpy(target, &result, inst->size);
}

void execute_dec(machine_state* state, instruction* inst)
{
    byte* target = resolve_operand(state, &inst->operands[0]);
    u64 result = --*(u64*)target;
    memcpy(target, &result, inst->size);
}

void execute_cmp(machine_state* state, instruction* inst)
{
    u64 left = *(u64*)resolve_operand(state, &inst->operands[0]);
    u64 right = *(u64*)resolve_operand(state, &inst->operands[1]);
    u64 result = left - right;
    memcpy(&state->registers[RFLAG], &result, inst->size);
}

void execute_je(machine_state* state, instruction* inst)
{
    if ((i64)state->registers[RFLAG] == 0)
    {
        jump(state, inst);
    }
}

void execute_jne(machine_state* state, instruction* inst)
{
    if ((i64)state->registers[RFLAG] != 0)
    {
        jump(state, inst);
    }
}

void execute_jl(machine_state* state, instruction* inst)
{
    if ((i64)state->registers[RFLAG] < 0)
    {
        jump(state, inst);
    }
}

void execute_jle(machine_state* state, instruction* inst)
{
    if ((i64)state->registers[RFLAG] <= 0)
    {
        jump(state, inst);
    }
}

void execute_jg(machine_state* state, instruction* inst)
{
    if ((i64)state->registers[RFLAG] > 0)
    {
        jump(state, inst);
    }
}

void execute_jge(machine_state* state, instruction* inst)
{
    if ((i64)state->registers[RFLAG] >= 0)
    {
        jump(state, inst);
    }
}

bool execute(machine_state* state)
{
    instruction inst;

    state->registers[RIP] += instruction_decode(
            state->memory + state->registers[RIP],
            &inst);

    instruction_print(&inst);

    void (*func)(machine_state* state, instruction* inst);

    switch (inst.opcode)
    {
        case OP_JMP:  func = execute_jmp;  break;
        case OP_PUSH: func = execute_push; break;
        case OP_POP:  func = execute_pop;  break;
        case OP_MOV:  func = execute_mov;  break;
        case OP_CALL: func = execute_call; break;
        case OP_RET:  func = execute_ret;  break;
        case OP_ADD:  func = execute_add;  break;
        case OP_SUB:  func = execute_sub;  break;
        case OP_MUL:  func = execute_mul;  break;
        case OP_DIV:  func = execute_div;  break;
        case OP_MOD:  func = execute_mod;  break;
        case OP_INC:  func = execute_inc;  break;
        case OP_DEC:  func = execute_dec;  break;
        case OP_CMP:  func = execute_cmp;  break;
        case OP_JE:   func = execute_je;   break;
        case OP_JNE:  func = execute_jne;  break;
        case OP_JG:   func = execute_jg;   break;
        case OP_JGE:  func = execute_jge;  break;
        case OP_JL:   func = execute_jl;   break;
        case OP_JLE:  func = execute_jle;  break;

        case OP_EXIT:
            return false;

        default:
            printf("Unrecognized opcode\n");
            exit(3);
    }

    func(state, &inst);

    return true;
}

void print_debug(machine_state* state)
{
    printf("  r0: %llu", state->registers[R0]);
    printf("  r1: %llu", state->registers[R1]);
    printf("  rflag: %lli", state->registers[RFLAG]);
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
