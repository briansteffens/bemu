#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "shared.h"

typedef struct bstring
{
    int len;
    unsigned char* data;
} bstring;

typedef struct bstring_array
{
    unsigned short allocated;
    unsigned short len;
    bstring* items;
} bstring_array;

typedef struct label
{
    bstring name;
    unsigned address;
} label;

typedef struct label_array
{
    unsigned short allocated;
    unsigned short len;
    label* items;
} label_array;

typedef struct jump
{
    unsigned short inst_index;
    bstring label_name;
} jump;

typedef struct jump_array
{
    unsigned short allocated;
    unsigned short len;
    jump* items;
} jump_array;

void jump_array_new(jump_array* target)
{
    target->allocated = 1;
    target->len = 0;
    target->items = malloc(sizeof(jump) * target->allocated);
}

jump* jump_array_add(jump_array* target)
{
    if (target->len == target->allocated)
    {
        target->allocated *= 2;
        target->items = realloc(target->items,
                sizeof(jump) * target->allocated);
    }

    target->len++;
    return &target->items[target->len - 1];
}

void label_array_new(label_array* target)
{
    target->allocated = 1;
    target->len = 0;
    target->items = malloc(sizeof(label) * target->allocated);
}

label* label_array_add(label_array* target)
{
    if (target->len == target->allocated)
    {
        target->allocated *= 2;
        target->items = realloc(target->items,
                sizeof(label) * target->allocated);
    }

    target->len++;
    return &target->items[target->len - 1];
}

bstring bstring_from_char(char* src)
{
    bstring ret;

    ret.data = src;
    ret.len = strlen(src);

    return ret;
}

bstring bstring_clone(bstring* src)
{
    bstring ret;

    ret.data = src->data;
    ret.len = src->len;

    return ret;
}

int bstring_chr(bstring* haystack, char needle)
{
    for (int i = 0; i < haystack->len; i++)
    {
        if (*(haystack->data + i) == needle)
        {
            return i;
        }
    }

    return -1;
}

bool bstring_cmp(bstring left, bstring right)
{
    if (left.len != right.len)
    {
        return false;
    }

    for (int i = 0; i < left.len; i++)
    {
        if (*(left.data + i) != *(right.data + i))
        {
            return false;
        }
    }

    return true;
}

void bstring_print(bstring* target)
{
    for (int i = 0; i < target->len; i++)
    {
        putchar(target->data[i]);
    }
}

void bstring_array_print(bstring_array* target)
{
    for (int i = 0; i < target->len; i++)
    {
        bstring_print(&target->items[i]);
        putchar('\n');
    }
}

bool is_whitespace(char c)
{
    return c == '\n' || c == ' ' || c == '\t';
}

void bstring_trim(bstring* target)
{
    while (target->len > 0 && is_whitespace(*target->data))
    {
        target->data++;
        target->len--;
    }

    while (target->len > 0 && is_whitespace(*(target->data + target->len - 1)))
    {
        target->len--;
    }
}

void bstring_array_new(bstring_array* target)
{
    target->allocated = 1;
    target->len = 0;
    target->items = malloc(sizeof(bstring) * target->allocated);
}

bstring* bstring_array_add(bstring_array* target)
{
    if (target->len == target->allocated)
    {
        target->allocated *= 2;
        target->items = realloc(target->items,
                sizeof(bstring) * target->allocated);
    }

    return &target->items[target->len++];
}

bool split(bstring* in, char separator, bstring_array* out)
{
    bstring_array_new(out);

    unsigned char* start = NULL;
    for (int i = 0; i < in->len; i++)
    {
        if (!start)
        {
            start = in->data + i;
        }

        if (in->data[i] != separator && i < in->len - 1)
        {
            continue;
        }

        int len = (in->data + i) - start;
        if (len == 0)
        {
            continue;
        }

        bstring* inst = bstring_array_add(out);
        inst->data = start;
        inst->len = (in->data + i) - start;

        if (i == in->len - 1)
        {
            inst->len++;
        }

        bstring_trim(inst);

        start = NULL;
    }

    return true;
}

unsigned char opcode_from_bstring(bstring src)
{
    if (bstring_cmp(src, bstring_from_char("push")))
    {
        return OP_PUSH;
    }
    else if (bstring_cmp(src, bstring_from_char("pop")))
    {
        return OP_POP;
    }
    else if (bstring_cmp(src, bstring_from_char("jmp")))
    {
        return OP_JMP;
    }
    else if (bstring_cmp(src, bstring_from_char("exit")))
    {
        return OP_EXIT;
    }

    printf("Unrecognized opcode\n");
    exit(6);
}

unsigned char register_from_bstring(bstring src)
{
    if (bstring_cmp(src, bstring_from_char("r0")))
    {
        return R0;
    }
    if (bstring_cmp(src, bstring_from_char("r1")))
    {
        return R1;
    }
    else if (bstring_cmp(src, bstring_from_char("r2")))
    {
        return R2;
    }
    else if (bstring_cmp(src, bstring_from_char("r3")))
    {
        return R3;
    }
    else if (bstring_cmp(src, bstring_from_char("r4")))
    {
        return R4;
    }
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
        out->data = register_from_bstring(in);
        out->type = REGISTER;
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

bool is_label(bstring_array* parts)
{
    return (parts->len == 1 &&
            parts->items[0].data[parts->items[0].len - 1] == ':');
}

void parse_instructions(
        bstring_array* lines,
        instruction_array* instructions,
        label_array* labels,
        jump_array* jumps)
{
    unsigned offset = 0;

    for (int i = 0; i < lines->len; i++)
    {
        bstring* line = &lines->items[i];

        if (line->len == 0)
        {
            continue;
        }

        bstring_array parts;
        split(line, ' ', &parts);

        // Label?
        if (is_label(&parts))
        {
            label* lbl = label_array_add(labels);

            lbl->name.data = parts.items[0].data;
            lbl->name.len = parts.items[0].len - 1;

            lbl->address = offset;

            goto next;
        }

        instruction* inst = instruction_array_add(instructions);

        inst->opcode = opcode_from_bstring(parts.items[0]);
        inst->size = B8;

        // Jumps need their labels resolved in a subsequent pass
        if (inst->opcode == OP_JMP)
        {
            jump* jmp = jump_array_add(jumps);

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

label* label_array_find(label_array* labels, bstring name)
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
        instruction_array* instructions,
        label_array* labels,
        jump_array* jumps)
{
    for (int i = 0; i < jumps->len; i++)
    {
        jump* jmp = &jumps->items[i];

        label* label = label_array_find(labels, jmp->label_name);
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

void operand_encode(operand* oper, unsigned char* out)
{
    *(out++) = oper->type | oper->mode << 1;
    encode_u64(oper->data, out);
}

int instruction_encode(instruction* inst, unsigned char* out)
{
    unsigned char* src = out;

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
        instruction_array* instructions,
        label_array* labels,
        unsigned char* bytes)
{
    unsigned char* out = bytes;

    int next_label_index = 0;
    label* next_label = NULL;

    for (int i = 0; i < instructions->len; i++)
    {
        out += instruction_encode(&instructions->items[i], out);
    }

    return out - bytes;
}

int get_entry_point(label_array* labels)
{
    label* start = label_array_find(labels, bstring_from_char("start"));

    return start ? start->address : 0;
}

void write_to_file(unsigned char* bytes, int count, const char* filename)
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

unsigned char* assemble(bstring* raw, int* out_bytes_count)
{
    bstring_array lines;
    split(raw, '\n', &lines);

    instruction_array instructions;
    instruction_array_new(&instructions);

    label_array labels;
    label_array_new(&labels);

    jump_array jumps;
    jump_array_new(&jumps);

    parse_instructions(&lines, &instructions, &labels, &jumps);

    resolve_jumps(&instructions, &labels, &jumps);

    unsigned char* bytes = malloc(sizeof(unsigned char) *
            IMG_HDR_LEN + MAX_ENCODED_INSTRUCTION_LEN * instructions.len);

    unsigned long long code_bytes = encode(
            &instructions,
            &labels,
            bytes + IMG_HDR_LEN);

    unsigned long long entry_point = get_entry_point(&labels);

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
    raw.data = read_file(argv[1], &raw.len);

    int bytes_len = 0;
    unsigned char* bytes = assemble(&raw, &bytes_len);

    free(raw.data);

    write_to_file(bytes, bytes_len, "b.out");

    free(bytes);

    return 0;
}
