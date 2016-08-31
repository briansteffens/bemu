#include <stdio.h>

#include "assembler.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: basm <source_file>\n");
        return 1;
    }

    operands_init();

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
