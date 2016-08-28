#include <stdbool.h>

extern const int OPERAND_BYTES_LEN;
extern const int DEBUG_STR_LEN;
extern const int MAX_ENCODED_INSTRUCTION_LEN;

extern const int IMG_HDR_LEN;
extern const int IMG_HDR_CODE_BYTES;
extern const int IMG_HDR_ENTRY_POINT;

enum opcodes
{
    OP_PUSH,
    OP_POP,
    OP_JMP
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
    R1 = 1,
    R2,
    R3,
    R4
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
    unsigned char type;
    unsigned char mode;
    long long data;
} operand;

typedef struct instruction
{
    unsigned char opcode;
    unsigned char size;
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

int operand_decode(unsigned char* in, operand* oper);

int operands(unsigned char opcode);

int instruction_decode(unsigned char* in, instruction* out);

const char* operand_type_to_string(unsigned char type);

const char* operand_mode_to_string(unsigned char mode);

void operand_to_string(operand* oper, char* out);

const char* opcode_to_string(unsigned char opcode);

const char* size_to_string(unsigned char size);

void instruction_print(instruction* inst);

void instruction_array_print(instruction_array* target);

unsigned char* read_file(const char* filename, int* out_bytes_read);
