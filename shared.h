#include <stdbool.h>

#define OPERAND_BYTES_LEN 9
#define DEBUG_STR_LEN 255
#define MAX_ENCODED_INSTRUCTION_LEN 2 + 9 * 2

#define IMG_HDR_LEN 16
#define IMG_HDR_CODE_BYTES 0
#define IMG_HDR_ENTRY_POINT 8

#define REGISTER_COUNT 7

typedef unsigned long long u64;
typedef unsigned char byte;

enum opcodes
{
    OP_PUSH,
    OP_POP,
    OP_JMP,
    OP_EXIT,
    OP_MOV,
    OP_CALL,
    OP_RET
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
    RSP
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

typedef struct instruction_array
{
    unsigned short allocated;
    unsigned short len;
    instruction* items;
} instruction_array;

void instruction_array_new(instruction_array* target);

instruction* instruction_array_add(instruction_array* target);

int operand_decode(byte* in, operand* oper);

int operands(byte opcode);

int instruction_decode(byte* in, instruction* out);

const char* operand_type_to_string(byte type);

const char* operand_mode_to_string(byte mode);

void operand_to_string(operand* oper, char* out);

const char* opcode_to_string(byte opcode);

const char* size_to_string(byte size);

void instruction_print(instruction* inst);

void instruction_array_print(instruction_array* target);

byte* read_file(const char* filename, int* out_bytes_read);

void encode_u64(u64 in, byte* out);

byte operand_unpack_register(operand* oper);

byte operand_unpack_offset(operand* oper);

void u64_debug_print(u64 in);
