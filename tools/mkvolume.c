#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE     4096
#define PAGE_SIZE      4096
#define RESERVED_PAGES 64

static void append(FILE* dst, const char* path)
{
    FILE*  src;
    char   buffer[BUFFERSIZE];
    size_t size;

    src = fopen(path, "rb");
    for (;;)
    {
        size = fread(buffer, 1, BUFFERSIZE, src);
        if (!size)
            break;
        fwrite(buffer, size, 1, dst);
    }
    fclose(src);
}

static void resize(FILE* f, int size)
{
    char zero;

    /* Adjust the size so it's a multiple of the page size */
    size = ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
    fseek(f, size - 1, SEEK_SET);
    zero = 0;
    fwrite(&zero, 1, 1, f);
}

static void add_vbr(FILE* f, const char* path)
{
    fseek(f, 0, SEEK_SET);
    append(f, path);
}

int main(int argc, char** argv)
{
    FILE* f;

    f = fopen(argv[1], "wb");
    resize(f, atoi(argv[2]));
    add_vbr(f, argv[3]);
    fclose(f);
    return 0;
}
