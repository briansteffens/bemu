#include "shared.h"

void operand_to_string(instruction* inst, int operand_ordinal, char* out);

const char* opcode_to_string(byte opcode);

const char* size_to_string(byte size);

const char* register_to_string(byte r);
