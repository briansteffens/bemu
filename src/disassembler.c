#include <stdlib.h>

#include "disassembler.h"

void operand_to_string(instruction* inst, int ordinal, char* out)
{
    byte type = inst->operand_types[ordinal];

    if (type & ADDRESS)
    {
        *(out++) = '[';
    }

    if (type & IMMEDIATE)
    {
        out += snprintf(out, 32, "%llu", inst->operands[ordinal]);
    }
    else if (type & REGISTER)
    {
        complex_operand* comp = (complex_operand*)&inst->operands[ordinal];

        out += snprintf(out, 32, "%s", register_to_string(comp->base));

        if (comp->multiplier != 1)
        {
            out += snprintf(out, 4, "*%d", comp->multiplier);
        }

        if (comp->register2_sign != 0)
        {
            out += snprintf(out, 32, "%c%s", comp->register2_sign ? '+' : '-',
                    register_to_string(comp->register2));
        }

        if (comp->offset > 0)
        {
            out += snprintf(out, 32, "%c%d", comp->offset ? '+' : '-',
                            comp->offset);
        }
    }

    if (type & ADDRESS)
    {
        *(out++) = ']';
    }

    *out = 0;
}

const char* opcode_to_string(byte opcode)
{
    switch (opcode)
    {
        case OP_PUSH:   return "push";
        case OP_POP:    return "pop";
        case OP_JMP:    return "jmp";
        case OP_EXIT:   return "exit";
        case OP_MOV:    return "mov";
        case OP_CALL:   return "call";
        case OP_RET:    return "ret";
        case OP_ADD:    return "add";
        case OP_SUB:    return "sub";
        case OP_MUL:    return "mul";
        case OP_DIV:    return "div";
        case OP_MOD:    return "mod";
        case OP_INC:    return "inc";
        case OP_DEC:    return "dec";
        case OP_CMP:    return "cmp";
        case OP_JE:     return "je";
        case OP_JNE:    return "jne";
        case OP_JL:     return "jl";
        case OP_JG:     return "jg";
        case OP_JLE:    return "jle";
        case OP_JGE:    return "jge";
        case OP_PRINT:  return "print";

        default:
            printf("Unrecognized opcode\n");
            exit(3);
    }
}

const char* size_to_string(byte size)
{
    switch (size)
    {
        case B1:
            return "B1";

        case B2:
            return "B2";

        case B4:
            return "B4";

        case B8:
            return "B8";

        default:
            printf("Unrecognized size\n");
            exit(5);
    }
}

const char* register_to_string(byte r)
{
    switch (r)
    {
        case R0:    return "r0";
        case R1:    return "r1";
        case R2:    return "r2";
        case R3:    return "r3";
        case R4:    return "r4";
        case R5:    return "r5";
        case RIP:   return "rip";
        case RSP:   return "rsp";
        case RFLAG: return "rflag";
        case RMEM:  return "rmem";

        default:
            printf("Unrecognized register\n");
            exit(13);
    }
}
