#include "shared.h"

#define MEMORY_SIZE 32 * 1024 * 1024

typedef struct
{
    u64 registers[REGISTER_COUNT];
    byte* memory;
} machine_state;

int read_next_instruction(machine_state* state, instruction** inst);

bool execute(machine_state* state);

void load_binary(const char* fn, machine_state* state);
