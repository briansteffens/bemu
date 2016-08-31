#ifndef _SHARED_H
#define _SHARED_H

#include "vector.h"

#define DEBUG_STR_LEN 255

#define IMG_HDR_LEN 16
#define IMG_HDR_CODE_BYTES 0
#define IMG_HDR_ENTRY_POINT 8

#define REGISTER_COUNT 10
#define MAX_OPERANDS 2

typedef unsigned long long u64;
typedef long long i64;
typedef unsigned char byte;

enum opcodes
{
    OP_PUSH,
    OP_POP,
    OP_JMP,
    OP_EXIT,
    OP_MOV,
    OP_CALL,
    OP_RET,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_INC,
    OP_DEC,
    OP_CMP,
    OP_JE,
    OP_JNE,
    OP_JL,
    OP_JG,
    OP_JLE,
    OP_JGE,
    OP_PRINT
};

enum sizes
{
    B1 = 1,
    B2 = 2,
    B4 = 4,
    B8 = 8
};

enum registers
{
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    RIP,
    RSP,
    RFLAG,
    RMEM
};

enum operand_type
{
    IMMEDIATE = 1,
    REGISTER  = 2,
    LITERAL   = 4,
    ADDRESS   = 8,
    COMPLEX   = 16
};

typedef struct instruction
{
    byte opcode;
    byte size;
    byte operand_types[MAX_OPERANDS];
    u64 operands[MAX_OPERANDS];
} instruction;

typedef struct
{
    unsigned char base;
    unsigned char multiplier;
    char register2_sign;
    unsigned char register2;
    int offset;
} complex_operand;

VECTOR_H(instruction);

int operands(byte opcode);

byte* read_file(const char* fn, byte* out_bytes, int* out_bytes_read);

#endif
