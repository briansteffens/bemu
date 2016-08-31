#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "emulator.h"

char* print_prefix = "";
char* print_suffix = "";

byte* resolve_operand(machine_state* state, instruction* inst, int ordinal)
{
    byte type = inst->operand_types[ordinal];
    complex_operand* comp = (complex_operand*)&inst->operands[ordinal];

    if (type & COMPLEX)
    {
        u64 addr = state->registers[comp->base];

        if (comp->multiplier)
        {
            addr *= comp->multiplier;
        }

        if (comp->register2_sign != 0)
        {
            u64 register2_val = state->registers[comp->register2];

            if (comp->register2_sign)
            {
                addr += register2_val;
            }
            else
            {
                addr -= register2_val;
            }
        }

        if (comp->offset != 0)
        {
            addr += comp->offset;
        }

        return &state->memory[addr];
    }

    u64* ret = NULL;

    if (type & IMMEDIATE)
    {
        ret = &inst->operands[ordinal];
    }
    else if (type & REGISTER)
    {
        ret = &state->registers[comp->base];
    }

    if (type & ADDRESS)
    {
        return &state->memory[*ret];
    }
    else
    {
        return (byte*)ret;
    }
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
    state->registers[RIP] =
        IMG_HDR_LEN + *(u64*)resolve_operand(state, inst, 0);
}

void execute_jmp(machine_state* state, instruction* inst)
{
    jump(state, inst);
}

void execute_push(machine_state* state, instruction* inst)
{
    push(state, resolve_operand(state, inst, 0), inst->size);
}

void execute_pop(machine_state* state, instruction* inst)
{
    pop(state, resolve_operand(state, inst, 0), inst->size);
}

void execute_mov(machine_state* state, instruction* inst)
{
    memcpy(resolve_operand(state, inst, 0),
           resolve_operand(state, inst, 1),
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
    byte* target = resolve_operand(state, inst, 0);
    u64 left = *(u64*)target;
    u64 right = *(u64*)resolve_operand(state, inst, 1);
    u64 result = handler(left, right);
    memcpy(target, &result, inst->size);
}

#define math_handler(name, op)                                                \
    u64 math_handler_##name(u64 left, u64 right)                              \
    {                                                                         \
        return left op right;                                                 \
    }                                                                         \
    void execute_##name(machine_state* state, instruction* inst)              \
    {                                                                         \
        basic_math(state, inst, math_handler_##name);                         \
    }                                                                         \

math_handler(add, +)
math_handler(sub, -)
math_handler(mul, *)
math_handler(div, /)
math_handler(mod, %)

void execute_inc(machine_state* state, instruction* inst)
{
    byte* target = resolve_operand(state, inst, 0);
    u64 result = ++*(u64*)target;
    memcpy(target, &result, inst->size);
}

void execute_dec(machine_state* state, instruction* inst)
{
    byte* target = resolve_operand(state, inst, 0);
    u64 result = --*(u64*)target;
    memcpy(target, &result, inst->size);
}

void execute_cmp(machine_state* state, instruction* inst)
{
    u64 left = *(u64*)resolve_operand(state, inst, 0);
    u64 right = *(u64*)resolve_operand(state, inst, 1);
    u64 result = left - right;
    memcpy(&state->registers[RFLAG], &result, inst->size);
}

#define conditional_handler(name, cmp)                                        \
    void execute_##name(machine_state* state, instruction* inst)              \
    {                                                                         \
        if ((i64)state->registers[RFLAG] cmp 0)                               \
        {                                                                     \
            jump(state, inst);                                                \
        }                                                                     \
    }

conditional_handler(je,  ==)
conditional_handler(jne, !=)
conditional_handler(jl,  < )
conditional_handler(jle, <=)
conditional_handler(jg,  > )
conditional_handler(jge, >=)

void execute_print(machine_state* state, instruction* inst)
{
    printf("%s%llu%s\n",
           print_prefix,
           *(u64*)resolve_operand(state, inst, 0),
           print_suffix);
}

int read_next_instruction(machine_state* state, instruction** inst)
{
    *inst = (instruction*)(state->memory + state->registers[RIP]);

    return operands((*inst)->opcode) * sizeof(u64) + 8;
}

bool execute_instruction(machine_state* state, instruction* inst)
{
    void (*func)(machine_state* state, instruction* inst);

    switch (inst->opcode)
    {
        case OP_JMP:   func = execute_jmp;   break;
        case OP_PUSH:  func = execute_push;  break;
        case OP_POP:   func = execute_pop;   break;
        case OP_MOV:   func = execute_mov;   break;
        case OP_CALL:  func = execute_call;  break;
        case OP_RET:   func = execute_ret;   break;
        case OP_ADD:   func = execute_add;   break;
        case OP_SUB:   func = execute_sub;   break;
        case OP_MUL:   func = execute_mul;   break;
        case OP_DIV:   func = execute_div;   break;
        case OP_MOD:   func = execute_mod;   break;
        case OP_INC:   func = execute_inc;   break;
        case OP_DEC:   func = execute_dec;   break;
        case OP_CMP:   func = execute_cmp;   break;
        case OP_JE:    func = execute_je;    break;
        case OP_JNE:   func = execute_jne;   break;
        case OP_JG:    func = execute_jg;    break;
        case OP_JGE:   func = execute_jge;   break;
        case OP_JL:    func = execute_jl;    break;
        case OP_JLE:   func = execute_jle;   break;
        case OP_PRINT: func = execute_print; break;

        case OP_EXIT:
            return false;

        default:
            printf("Unrecognized opcode\n");
            exit(3);
    }

    func(state, inst);

    return true;
}

bool execute(machine_state* state)
{
    instruction* inst;
    state->registers[RIP] += read_next_instruction(state, &inst);
    return execute_instruction(state, inst);
}

void load_binary(const char* fn, machine_state* state)
{
    state->memory = malloc(sizeof(byte) * MEMORY_SIZE);

    int bytes_count;
    read_file(fn, state->memory, &bytes_count);

    // Clear registers
    for (int i = 0; i < REGISTER_COUNT; i++)
    {
        state->registers[i] = 0;
    }

    // Align rmem on 8-byte boundary after image
    state->registers[RMEM] = bytes_count + 8 - bytes_count % 8;

    state->registers[RIP] =
        IMG_HDR_LEN + *(u64 *)(state->memory + IMG_HDR_ENTRY_POINT);

    state->registers[RSP] = MEMORY_SIZE;
}
