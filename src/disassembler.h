#include "shared.h"

void operand_to_string(instruction* inst, int operand_ordinal, char* out);

const char* opcode_to_string(unsigned char opcode);

const char* size_to_string(unsigned char size);

const char* register_to_string(unsigned char r);
