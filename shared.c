#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "shared.h"

void instruction_array_new(instruction_array* target)
{
    target->allocated = 1;
    target->len = 0;
    target->items = malloc(sizeof(instruction) * target->allocated);
}

instruction* instruction_array_add(instruction_array* target)
{
    target->len++;

    if (target->len > target->allocated)
    {
        target->allocated *= 2;
        target->items = realloc
        (
            target->items,
            sizeof(instruction) * target->allocated
        );
    }

    return &target->items[target->len - 1];
}

int operand_decode(byte* in, operand* oper)
{
    oper->type = (*in & (1 << 0)) >> 0;
    oper->mode = (*in & (1 << 1)) >> 1;
    oper->data = *(long long *)(in + 1);

    return 9;
}

int operands(byte opcode)
{
    switch (opcode)
    {
        case OP_MOV:
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
        case OP_CMP:
            return 2;

        case OP_PUSH:
        case OP_POP:
        case OP_JMP:
        case OP_CALL:
        case OP_INC:
        case OP_DEC:
        case OP_JE:
        case OP_JNE:
        case OP_JL:
        case OP_JG:
        case OP_JLE:
        case OP_JGE:
            return 1;

        case OP_EXIT:
        case OP_RET:
            return 0;

        default:
            printf("Unrecognized opcode\n");
            exit(4);
    }
}

int instruction_decode(byte* in, instruction* out)
{
    byte* src = in;

    out->opcode = *(src++);
    out->size = *(src++);

    int operand_count = operands(out->opcode);

    for (int i = 0; i < operand_count; i++)
    {
        src += operand_decode(src, &out->operands[i]);
    }

    return src - in;
}

const char* operand_type_to_string(byte type)
{
    switch (type)
    {
        case IMMEDIATE:
            return "IMMEDIATE";

        case REGISTER:
            return "REGISTER";

        default:
            printf("Unrecognized operand type\n");
            exit(1);
    }
}

const char* operand_mode_to_string(byte mode)
{
    switch (mode)
    {
        case LITERAL:
            return "LITERAL";

        case ADDRESS:
            return "ADDRESS";

        default:
            printf("Unrecognized operand mode\n");
            exit(2);
    }
}

void operand_to_string(operand* oper, char* out)
{
    snprintf
    (
        out,
        DEBUG_STR_LEN,
        "%s %s %llu",
        operand_type_to_string(oper->type),
        operand_mode_to_string(oper->mode),
        oper->data
    );
}

const char* opcode_to_string(byte opcode)
{
    switch (opcode)
    {
        case OP_PUSH: return "push";
        case OP_POP:  return "pop";
        case OP_JMP:  return "jmp";
        case OP_EXIT: return "exit";
        case OP_MOV:  return "mov";
        case OP_CALL: return "call";
        case OP_RET:  return "ret";
        case OP_ADD:  return "add";
        case OP_SUB:  return "sub";
        case OP_MUL:  return "mul";
        case OP_DIV:  return "div";
        case OP_MOD:  return "mod";
        case OP_INC:  return "inc";
        case OP_DEC:  return "dec";
        case OP_CMP:  return "cmp";
        case OP_JE:   return "je";
        case OP_JNE:  return "jne";
        case OP_JL:   return "jl";
        case OP_JG:   return "jg";
        case OP_JLE:  return "jle";
        case OP_JGE:  return "jge";

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

void instruction_print(instruction* inst)
{
    printf
    (
        "%s %s",
        opcode_to_string(inst->opcode),
        size_to_string(inst->size)
    );

    int operand_count = operands(inst->opcode);
    char buffer[DEBUG_STR_LEN];

    for (int i = 0; i < operand_count; i++)
    {
        operand_to_string(&inst->operands[i], buffer);
        printf(" %s", buffer);
    }

    putchar('\n');
}

void instruction_array_print(instruction_array* target)
{
    for (int i = 0; i < target->len; i++)
    {
        instruction_print(&target->items[i]);
    }
}

byte* read_file(const char* filename, int* out_bytes_read)
{
    FILE* file = fopen(filename, "r");

    if (!file)
    {
        printf("Failed to open file.\n");
        return false;
    }

    struct stat file_stat;
    if (fstat(fileno(file), &file_stat) < 0)
    {
        printf("Failed to stat file.\n");
        return false;
    }

    byte* ret = malloc(sizeof(byte) * file_stat.st_size);

    if (!fread(ret, file_stat.st_size, 1, file))
    {
        printf("Failed to read file contents.\n");
        return false;
    }

    fclose(file);

    *out_bytes_read = file_stat.st_size;

    return ret;
}

void encode_u64(u64 in, byte* out)
{
    for (int shift = 0; shift <= 56; shift += 8)
    {
        *(out++) = (in >> shift) & 0xff;
    }
}

byte operand_unpack_register(operand* oper)
{
    return *(byte*)(&oper->data);
}

byte operand_unpack_offset(operand* oper)
{
    return oper->data >> 32;
}

void u64_debug_print(u64 in)
{
    void* p = (void*)&in;

    for (int i = 0; i < 8; ++i)
    {
        printf("%02X ", ((unsigned char*)p)[i]);
    }

    putchar('\n');
}
