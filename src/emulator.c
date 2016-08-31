#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "emulator.h"

bool (*opcode_handlers[OPCODE_COUNT])(machine_state* state, instruction* inst);

char* print_prefix = "";
char* print_suffix = "";

byte* resolve_operand(machine_state* state, instruction* inst, int ordinal)
{
    byte type = inst->operand_types[ordinal];
    complex_operand* comp = (complex_operand*)&inst->operands[ordinal];

    if (type & COMPLEX)
    {
        u64 addr = state->registers[comp->base];

        addr *= comp->multiplier;

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

        addr += comp->offset;

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

bool push(machine_state* state, byte* data, byte size)
{
    state->registers[RSP] -= size;

    memcpy(state->memory + state->registers[RSP], data, size);

    return true;
}

bool pop(machine_state* state, byte* target, byte size)
{
    memcpy(target, state->memory + state->registers[RSP], size);

    state->registers[RSP] += size;

    return true;
}

bool jump(machine_state* state, instruction* inst)
{
    state->registers[RIP] =
        IMG_HDR_LEN + *(u64*)resolve_operand(state, inst, 0);

    return true;
}

bool execute_jmp(machine_state* state, instruction* inst)
{
    jump(state, inst);

    return true;
}

bool execute_push(machine_state* state, instruction* inst)
{
    push(state, resolve_operand(state, inst, 0), inst->size);

    return true;
}

bool execute_pop(machine_state* state, instruction* inst)
{
    pop(state, resolve_operand(state, inst, 0), inst->size);

    return true;
}

bool execute_mov(machine_state* state, instruction* inst)
{
    memcpy(resolve_operand(state, inst, 0),
           resolve_operand(state, inst, 1),
           inst->size);

    return true;
}

bool execute_call(machine_state* state, instruction* inst)
{
    push(state, (byte*)&state->registers[RIP], B8);
    execute_jmp(state, inst);

    return true;
}

bool execute_ret(machine_state* state, instruction* inst)
{
    pop(state, (byte*)&state->registers[RIP], B8);

    return true;
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
    bool execute_##name(machine_state* state, instruction* inst)              \
    {                                                                         \
        basic_math(state, inst, math_handler_##name);                         \
        return true;                                                          \
    }                                                                         \

math_handler(add, +)
math_handler(sub, -)
math_handler(mul, *)
math_handler(div, /)
math_handler(mod, %)

bool execute_inc(machine_state* state, instruction* inst)
{
    byte* target = resolve_operand(state, inst, 0);
    u64 result = ++*(u64*)target;
    memcpy(target, &result, inst->size);

    return true;
}

bool execute_dec(machine_state* state, instruction* inst)
{
    byte* target = resolve_operand(state, inst, 0);
    u64 result = --*(u64*)target;
    memcpy(target, &result, inst->size);

    return true;
}

bool execute_cmp(machine_state* state, instruction* inst)
{
    u64 left = *(u64*)resolve_operand(state, inst, 0);
    u64 right = *(u64*)resolve_operand(state, inst, 1);
    u64 result = left - right;
    memcpy(&state->registers[RFLAG], &result, inst->size);

    return true;
}

#define conditional_handler(name, cmp)                                        \
    bool execute_##name(machine_state* state, instruction* inst)              \
    {                                                                         \
        if ((i64)state->registers[RFLAG] cmp 0)                               \
        {                                                                     \
            jump(state, inst);                                                \
        }                                                                     \
        return true;                                                          \
    }

conditional_handler(je,  ==)
conditional_handler(jne, !=)
conditional_handler(jl,  < )
conditional_handler(jle, <=)
conditional_handler(jg,  > )
conditional_handler(jge, >=)

bool execute_print(machine_state* state, instruction* inst)
{
    printf("%s%llu%s\n",
           print_prefix,
           *(u64*)resolve_operand(state, inst, 0),
           print_suffix);

    return true;
}

bool execute_exit(machine_state* state, instruction* inst)
{
    return false;
}

int read_next_instruction(machine_state* state, instruction** inst)
{
    *inst = (instruction*)(state->memory + state->registers[RIP]);

    return operands[(*inst)->opcode] * sizeof(u64) + 8;
}

void emulator_init()
{
    opcode_handlers[OP_JMP]   = execute_jmp;
    opcode_handlers[OP_PUSH]  = execute_push;
    opcode_handlers[OP_POP]   = execute_pop;
    opcode_handlers[OP_MOV]   = execute_mov;
    opcode_handlers[OP_CALL]  = execute_call;
    opcode_handlers[OP_RET]   = execute_ret;
    opcode_handlers[OP_ADD]   = execute_add;
    opcode_handlers[OP_SUB]   = execute_sub;
    opcode_handlers[OP_MUL]   = execute_mul;
    opcode_handlers[OP_DIV]   = execute_div;
    opcode_handlers[OP_MOD]   = execute_mod;
    opcode_handlers[OP_INC]   = execute_inc;
    opcode_handlers[OP_DEC]   = execute_dec;
    opcode_handlers[OP_CMP]   = execute_cmp;
    opcode_handlers[OP_JE]    = execute_je;
    opcode_handlers[OP_JNE]   = execute_jne;
    opcode_handlers[OP_JG]    = execute_jg;
    opcode_handlers[OP_JGE]   = execute_jge;
    opcode_handlers[OP_JL]    = execute_jl;
    opcode_handlers[OP_JLE]   = execute_jle;
    opcode_handlers[OP_PRINT] = execute_print;
    opcode_handlers[OP_EXIT]  = execute_exit;
}

bool execute_instruction(machine_state* state, instruction* inst)
{
    return opcode_handlers[inst->opcode](state, inst);
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
