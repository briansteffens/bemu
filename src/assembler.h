#ifndef _ASSEMBLER_H
#define _ASSEMBLER_H

#include "bstring.h"
#include "shared.h"

vec_bstring parse_instruction_header(bstring* line, instruction* inst);

void parse_instruction_operands(vec_bstring* parts, instruction* inst);

byte* assemble(bstring* raw, int* out_bytes_count);

void write_to_file(byte* bytes, int count, const char* filename);

#endif
