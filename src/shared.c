#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "shared.h"

VECTOR_C(instruction);

int operands[OPCODE_COUNT];

void operands_init()
{
    operands[OP_MOV]   = 2;
    operands[OP_ADD]   = 2;
    operands[OP_SUB]   = 2;
    operands[OP_MUL]   = 2;
    operands[OP_DIV]   = 2;
    operands[OP_MOD]   = 2;
    operands[OP_CMP]   = 2;

    operands[OP_PUSH]  = 1;
    operands[OP_POP]   = 1;
    operands[OP_JMP]   = 1;
    operands[OP_CALL]  = 1;
    operands[OP_INC]   = 1;
    operands[OP_DEC]   = 1;
    operands[OP_JE]    = 1;
    operands[OP_JNE]   = 1;
    operands[OP_JL]    = 1;
    operands[OP_JG]    = 1;
    operands[OP_JLE]   = 1;
    operands[OP_JGE]   = 1;
    operands[OP_PRINT] = 1;

    operands[OP_EXIT]  = 0;
    operands[OP_RET]   = 0;
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

int instruction_encoded_len(int operands)
{
    return 8 + operands * 8;
}
