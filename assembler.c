#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "bstring.h"
#include "bemu.h"

typedef struct label
{
    bstring name;
    unsigned address;
} label;

VECTOR_H(label);
VECTOR_C(label);

typedef struct jump
{
    unsigned short inst_index;
    bstring label_name;
} jump;

VECTOR_H(jump);
VECTOR_C(jump);

byte opcode_from_bstring(bstring src)
{
    if      (bstring_cmp(src, bstring_from_char("push"))) { return OP_PUSH; }
    else if (bstring_cmp(src, bstring_from_char("pop")))  { return OP_POP;  }
    else if (bstring_cmp(src, bstring_from_char("jmp")))  { return OP_JMP;  }
    else if (bstring_cmp(src, bstring_from_char("exit"))) { return OP_EXIT; }
    else if (bstring_cmp(src, bstring_from_char("mov")))  { return OP_MOV;  }
    else if (bstring_cmp(src, bstring_from_char("call"))) { return OP_CALL; }
    else if (bstring_cmp(src, bstring_from_char("ret")))  { return OP_RET;  }
    else if (bstring_cmp(src, bstring_from_char("add")))  { return OP_ADD;  }
    else if (bstring_cmp(src, bstring_from_char("sub")))  { return OP_SUB;  }
    else if (bstring_cmp(src, bstring_from_char("mul")))  { return OP_MUL;  }
    else if (bstring_cmp(src, bstring_from_char("div")))  { return OP_DIV;  }
    else if (bstring_cmp(src, bstring_from_char("mod")))  { return OP_MOD;  }
    else if (bstring_cmp(src, bstring_from_char("inc")))  { return OP_INC;  }
    else if (bstring_cmp(src, bstring_from_char("dec")))  { return OP_DEC;  }
    else if (bstring_cmp(src, bstring_from_char("cmp")))  { return OP_CMP;  }
    else if (bstring_cmp(src, bstring_from_char("je")))   { return OP_JE;   }
    else if (bstring_cmp(src, bstring_from_char("jne")))  { return OP_JNE;  }
    else if (bstring_cmp(src, bstring_from_char("jl")))   { return OP_JL;   }
    else if (bstring_cmp(src, bstring_from_char("jle")))  { return OP_JLE;  }
    else if (bstring_cmp(src, bstring_from_char("jg")))   { return OP_JG;   }
    else if (bstring_cmp(src, bstring_from_char("jge")))  { return OP_JGE;  }

    printf("Unrecognized opcode\n");
    exit(6);
}

byte register_from_bstring(bstring src)
{
    if      (bstring_cmp(src, bstring_from_char("r0")))  { return R0;  }
    else if (bstring_cmp(src, bstring_from_char("r1")))  { return R1;  }
    else if (bstring_cmp(src, bstring_from_char("r2")))  { return R2;  }
    else if (bstring_cmp(src, bstring_from_char("r3")))  { return R3;  }
    else if (bstring_cmp(src, bstring_from_char("r4")))  { return R4;  }
    else if (bstring_cmp(src, bstring_from_char("rip"))) { return RIP; }
    else if (bstring_cmp(src, bstring_from_char("rsp"))) { return RSP; }
    else
    {
        printf("Unrecognized register.\n");
        exit(8);
    }
}

void parse_operand(bstring in, operand* out)
{
    if (*in.data == '[' && *(in.data + in.len - 1) == ']')
    {
        out->mode = ADDRESS;
        in.data++;
        in.len -= 2;
    }
    else
    {
        out->mode = LITERAL;
    }

    if (*in.data == 'r')
    {
        out->type = REGISTER;

        int offset = 0;

        vec_bstring sections = vec_bstring_new();
        bstring_split(&in, "+-", &sections);

        byte register_id = register_from_bstring(sections.items[0]);

        if (sections.len == 2)
        {
            char buf[11];
            bstring_to_char(&sections.items[1], buf);

            offset = atoi(buf);

            if (*(in.data + sections.items[0].len) == '-')
            {
                offset *= -1;
            }
        }

        out->data = (u64)offset << 32 | register_id;
    }
    else
    {
        char buf[21];

        for (int i = 0; i < in.len; i++)
        {
            buf[i] = *(in.data + i);
        }

        buf[in.len] = 0;

        out->data = strtoll(buf, NULL, 0);
        out->type = IMMEDIATE;
    }
}

int instruction_encoded_bytes(instruction* inst)
{
    return 2 + operands(inst->opcode) * 9;
}

bool is_label(vec_bstring* parts)
{
    return (parts->len == 1 &&
            parts->items[0].data[parts->items[0].len - 1] == ':');
}

bool is_jump(byte opcode)
{
    return opcode == OP_JMP || opcode == OP_JE || opcode == OP_JNE ||
           opcode == OP_JL || opcode == OP_JLE ||
           opcode == OP_JG || opcode == OP_JGE;
}

void parse_instructions(
        vec_bstring* lines,
        vec_instruction* instructions,
        vec_label* labels,
        vec_jump* jumps)
{
    unsigned offset = 0;

    for (int i = 0; i < lines->len; i++)
    {
        bstring* line = &lines->items[i];

        if (line->len == 0)
        {
            continue;
        }

        vec_bstring parts = vec_bstring_new();
        bstring_split(line, " ", &parts);

        // Label?
        if (is_label(&parts))
        {
            label* lbl = vec_label_add(labels);

            lbl->name.data = parts.items[0].data;
            lbl->name.len = parts.items[0].len - 1;

            lbl->address = offset;

            goto next;
        }

        instruction* inst = vec_instruction_add(instructions);

        inst->opcode = opcode_from_bstring(parts.items[0]);
        inst->size = B8;

        // Jumps need their labels resolved in a subsequent pass
        if (is_jump(inst->opcode))
        {
            jump* jmp = vec_jump_add(jumps);

            jmp->inst_index = instructions->len - 1;
            jmp->label_name = bstring_clone(&parts.items[1]);

            goto next_and_offset;
        }

        int operand_count = operands(inst->opcode);

        if (operand_count != parts.len - 1)
        {
            printf("Invalid number of operands\n");
            exit(7);
        }

        for (int i = 1; i < parts.len; i++)
        {
            parse_operand(parts.items[i], &inst->operands[i - 1]);
        }

    next_and_offset:
        offset += instruction_encoded_bytes(inst);

    next:
        free(parts.items);
    }
}

label* vec_label_find(vec_label* labels, bstring name)
{
    for (int i = 0; i < labels->len; i++)
    {
        if (bstring_cmp(labels->items[i].name, name))
        {
            return &labels->items[i];
        }
    }

    return NULL;
}

void resolve_jumps(
        vec_instruction* instructions,
        vec_label* labels,
        vec_jump* jumps)
{
    for (int i = 0; i < jumps->len; i++)
    {
        jump* jmp = &jumps->items[i];

        label* label = vec_label_find(labels, jmp->label_name);
        if (!label)
        {
            printf("Label not found");
            exit(8);
        }

        operand* oper = &instructions->items[jmp->inst_index].operands[0];

        oper->type = IMMEDIATE;
        oper->mode = LITERAL;
        oper->data = label->address;
    }
}

void operand_encode(operand* oper, byte* out)
{
    *(out++) = oper->type | oper->mode << 1;
    encode_u64(oper->data, out);
}

int instruction_encode(instruction* inst, byte* out)
{
    byte* src = out;

    *(src++) = inst->opcode;
    *(src++) = inst->size;

    int operand_count = operands(inst->opcode);

    for (int i = 0; i < operand_count; i++)
    {
        operand_encode(&inst->operands[i], src);
        src += OPERAND_BYTES_LEN;
    }

    return src - out;
}

int encode(
        vec_instruction* instructions,
        vec_label* labels,
        byte* bytes)
{
    byte* out = bytes;

    int next_label_index = 0;
    label* next_label = NULL;

    for (int i = 0; i < instructions->len; i++)
    {
        out += instruction_encode(&instructions->items[i], out);
    }

    return out - bytes;
}

int get_entry_point(vec_label* labels)
{
    label* start = vec_label_find(labels, bstring_from_char("start"));

    return start ? start->address : 0;
}

void write_to_file(byte* bytes, int count, const char* filename)
{
    FILE* file = fopen(filename, "w");

    if (!file)
    {
        printf("Unable to open file [%s] for writing.\n");
        exit(9);
    }

    size_t written = fwrite(bytes, 1, count, file);

    if (written != count)
    {
        printf("Incorrect amount of data written. Output file corrupt?\n");
        exit(10);
    }

    fclose(file);
}

byte* assemble(bstring* raw, int* out_bytes_count)
{
    vec_bstring lines = vec_bstring_new();
    bstring_split(raw, "\n", &lines);

    vec_instruction instructions = vec_instruction_new();

    vec_label labels = vec_label_new();

    vec_jump jumps = vec_jump_new();

    parse_instructions(&lines, &instructions, &labels, &jumps);

    resolve_jumps(&instructions, &labels, &jumps);

    byte* bytes = malloc(sizeof(byte) *
            IMG_HDR_LEN + MAX_ENCODED_INSTRUCTION_LEN * instructions.len);

    u64 code_bytes = encode(&instructions, &labels, bytes + IMG_HDR_LEN);

    u64 entry_point = get_entry_point(&labels);

    encode_u64(code_bytes, bytes + IMG_HDR_CODE_BYTES);
    encode_u64(entry_point, bytes + IMG_HDR_ENTRY_POINT);

    free(jumps.items);
    free(labels.items);
    free(instructions.items);
    free(lines.items);

    *out_bytes_count = IMG_HDR_LEN + code_bytes;
    return bytes;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: basm <source_file>\n");
        return 1;
    }

    bstring raw;
    raw.data = NULL;
    raw.data = read_file(argv[1], NULL, &raw.len);

    int bytes_len = 0;
    byte* bytes = assemble(&raw, &bytes_len);

    free(raw.data);

    write_to_file(bytes, bytes_len, "b.out");

    free(bytes);

    return 0;
}
