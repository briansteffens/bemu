#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "bemu.h"

#define MEMORY_SIZE 32 * 1024 * 1024

typedef struct
{
    u64 registers[REGISTER_COUNT];
    byte* memory;
    bool debug;
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
    state->registers[RIP] =
        IMG_HDR_LEN + *(u64*)resolve_operand(state, &inst->operands[0]);
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

bool execute(machine_state* state)
{
    instruction inst;

    state->registers[RIP] += instruction_decode(
            state->memory + state->registers[RIP],
            &inst);

    if (state->debug)
    {
        instruction_print(&inst);
    }

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
    printf("  r2: %llu", state->registers[R2]);
    printf("  r3: %llu", state->registers[R3]);
    printf("  r4: %llu", state->registers[R4]);
    printf("  rflag: %lli", state->registers[RFLAG]);
    putchar('\n');
}

int print_usage()
{
    printf("Usage: bvm <binary_file> [--debug]\n");
    return 1;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        return print_usage();
    }

    machine_state state;

    state.debug = false;
    char* filename = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strncmp(argv[i], "--debug", 255) == 0)
        {
            state.debug = true;
            continue;
        }

        if (filename != NULL)
        {
            return print_usage();
        }

        filename = argv[i];
    }

    state.memory = malloc(sizeof(byte) * MEMORY_SIZE);
    int bytes_count;
    read_file(filename, state.memory, &bytes_count);

    // Clear registers
    for (int i = 0; i < REGISTER_COUNT; i++)
    {
        state.registers[i] = 0;
    }

    // Align rmem on 8-byte boundary after image
    state.registers[RMEM] = bytes_count + 8 - bytes_count % 8;

    state.registers[RIP] =
        IMG_HDR_LEN + *(u64 *)(state.memory + IMG_HDR_ENTRY_POINT);
    state.registers[RSP] = MEMORY_SIZE;

    while (execute(&state))
    {
        if (state.debug)
        {
            print_debug(&state);

            // Debug prompt
            char* input = NULL;
            size_t len;
            int read = getline(&input, &len, stdin);

            if (read != -1)
            {
                // handle command
            }

            free(input);
        }
    }

    print_debug(&state);

    free(state.memory);

    return 0;
}
