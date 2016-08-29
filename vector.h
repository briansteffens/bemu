#ifndef _VECTOR_H
#define _VECTOR_H

#include <stdbool.h>
#include <malloc.h>

#define VECTOR_H(t)                                                           \
    typedef struct                                                            \
    {                                                                         \
        int allocated;                                                        \
        int len;                                                              \
        t* items;                                                             \
    } vec_##t;                                                                \
                                                                              \
    vec_##t vec_##t##_new();                                                  \
    t* vec_##t##_add(vec_##t* vec);                                           \

#define VECTOR_C(t)                                                           \
    vec_##t vec_##t##_new()                                                   \
    {                                                                         \
        vec_##t ret;                                                          \
        ret.allocated = 1;                                                    \
        ret.len = 0;                                                          \
        ret.items = malloc(sizeof(t) * ret.allocated);                        \
        return ret;                                                           \
    }                                                                         \
                                                                              \
    t* vec_##t##_add(vec_##t* vec)                                            \
    {                                                                         \
        if (vec->len == vec->allocated)                                       \
        {                                                                     \
            vec->allocated *= 2;                                              \
            vec->items = realloc(vec->items, sizeof(t) * vec->allocated);     \
        }                                                                     \
        return &vec->items[vec->len++];                                       \
    }

#endif
