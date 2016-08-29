#ifndef BSTRING_H
#define BSTRING_H

#include "vector.h"

typedef struct bstring
{
    int len;
    unsigned char* data;
} bstring;

VECTOR_H(bstring)

bool is_whitespace(char c);

bstring bstring_from_char(char* src);

void bstring_to_char(bstring* src, char* dest);

bstring bstring_clone(bstring* src);

int bstring_chr(bstring* haystack, char needle);

bool bstring_cmp(bstring left, bstring right);

void bstring_print(bstring* target);

void vec_bstring_print(vec_bstring* target);

void bstring_trim(bstring* target);

bool bstring_split(bstring* in, const char* separators, vec_bstring* out);

#endif
