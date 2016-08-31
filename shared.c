#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "shared.h"

VECTOR_C(instruction);

int operands(byte opcode)
{
    // TODO
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
        case OP_PRINT:
            return 1;

        case OP_EXIT:
        case OP_RET:
            return 0;

        default:
            printf("Unrecognized opcode\n");
            exit(4);
    }
}

byte* read_file(const char* fn, byte* out_bytes, int* out_bytes_read)
{
    FILE* file = fopen(fn, "r");

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

    if (out_bytes == NULL)
    {
        out_bytes = malloc(sizeof(byte) * file_stat.st_size);
    }

    if (!fread(out_bytes, file_stat.st_size, 1, file))
    {
        printf("Failed to read file contents.\n");
        return false;
    }

    fclose(file);

    *out_bytes_read = file_stat.st_size;

    return out_bytes;
}
