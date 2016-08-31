#include <string.h>

#include "bstring.h"

VECTOR_C(bstring)

bool is_whitespace(char c)
{
    return c == '\n' || c == ' ' || c == '\t';
}

bstring bstring_from_char(char* src)
{
    bstring ret;

    ret.data = src;
    ret.len = strlen(src);

    return ret;
}

void bstring_to_char(bstring* src, char* dest)
{
    memcpy(dest, src->data, src->len);
    dest[src->len] = 0;
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

void vec_bstring_print(vec_bstring* target)
{
    for (int i = 0; i < target->len; i++)
    {
        bstring_print(&target->items[i]);
        putchar('\n');
    }
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

bool bstring_split(bstring* in, const char* separators, vec_bstring* out)
{
    int separators_len = strnlen(separators, 255);

    unsigned char* start = NULL;
    for (int i = 0; i < in->len; i++)
    {
        if (!start)
        {
            start = in->data + i;
        }

        if (i < in->len - 1)
        {
            bool found = false;

            for (int j = 0; j < separators_len; j++)
            {
                if (in->data[i] == separators[j])
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                continue;
            }
        }

        bstring* inst = vec_bstring_add(out);
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
