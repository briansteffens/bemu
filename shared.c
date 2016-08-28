#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "shared.h"

const int OPERAND_BYTES_LEN = 9;
const int DEBUG_STR_LEN = 255;
const int MAX_ENCODED_INSTRUCTION_LEN = 2 + 9 * 2;

const int IMG_HDR_LEN = 16;
const int IMG_HDR_CODE_BYTES = 0;
const int IMG_HDR_ENTRY_POINT = 8;

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

int operand_decode(unsigned char* in, operand* oper)
{
    oper->type = (*in & (1 << 0)) >> 0;
    oper->mode = (*in & (1 << 1)) >> 1;
    oper->data = *(long long *)(in + 1);

    return 9;
}

int operands(unsigned char opcode)
{
    switch (opcode)
    {
        case OP_PUSH:
        case OP_POP:
        case OP_JMP:
            return 1;

        default:
            printf("Unrecognized opcode\n");
            exit(4);
    }
}

int instruction_decode(unsigned char* in, instruction* out)
{
    unsigned char* src = in;

    out->opcode = *(src++);
    out->size = *(src++);

    int operand_count = operands(out->opcode);

    for (int i = 0; i < operand_count; i++)
    {
        src += operand_decode(src, &out->operands[i]);
    }

    return src - in - 1;
}

const char* operand_type_to_string(unsigned char type)
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

const char* operand_mode_to_string(unsigned char mode)
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

const char* opcode_to_string(unsigned char opcode)
{
    switch (opcode)
    {
        case OP_PUSH:
            return "push";

        case OP_POP:
            return "pop";

        case OP_JMP:
            return "jmp";

        default:
            printf("Unrecognized opcode\n");
            exit(3);
    }
}

const char* size_to_string(unsigned char size)
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

unsigned char* read_file(const char* filename, int* out_bytes_read)
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

    unsigned char* ret = malloc(sizeof(unsigned char) * file_stat.st_size);

    if (!fread(ret, file_stat.st_size, 1, file))
    {
        printf("Failed to read file contents.\n");
        return false;
    }

    fclose(file);

    *out_bytes_read = file_stat.st_size;

    return ret;
}
