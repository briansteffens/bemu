#include <stdbool.h>

#include "vector.h"

#define OPERAND_BYTES_LEN 9
#define DEBUG_STR_LEN 255
#define MAX_ENCODED_INSTRUCTION_LEN 2 + 9 * 2

#define IMG_HDR_LEN 16
#define IMG_HDR_CODE_BYTES 0
#define IMG_HDR_ENTRY_POINT 8

#define REGISTER_COUNT 8

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
    OP_JGE
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
    RIP,
    RSP,
    RFLAG
};

enum operand_types
{
    IMMEDIATE,
    REGISTER
};

enum addresing_modes
{
    LITERAL,
    ADDRESS
};

typedef struct operand
{
    byte type;
    byte mode;
    long long data;
} operand;

typedef struct instruction
{
    byte opcode;
    byte size;
    operand operands[2];
} instruction;

VECTOR_H(instruction);

int operand_decode(byte* in, operand* oper);

int operands(byte opcode);

int instruction_decode(byte* in, instruction* out);

const char* operand_type_to_string(byte type);

const char* operand_mode_to_string(byte mode);

void operand_to_string(operand* oper, char* out);

const char* opcode_to_string(byte opcode);

const char* size_to_string(byte size);

void instruction_print(instruction* inst);

void vec_instruction_print(vec_instruction* target);

byte* read_file(const char* fn, byte* out_bytes, int* out_bytes_read);

void encode_u64(u64 in, byte* out);

byte operand_unpack_register(operand* oper);

byte operand_unpack_offset(operand* oper);

void u64_debug_print(u64 in);