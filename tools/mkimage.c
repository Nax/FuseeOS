#include <stdio.h>
#define BUFFERSIZE  4096
#define MAX_SECTORS 63
#define MAX_HEADS   256

static void padCHS(FILE* f)
{
    char   zero;
    size_t n;

    n = ftell(f) % (MAX_SECTORS * MAX_HEADS * 512);
    if (!n)
        return;
    fseek(f, (MAX_SECTORS * MAX_HEADS * 512) - n - 1, SEEK_CUR);
    zero = 0;
    fwrite(&zero, 1, 1, f);
}

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

static void makeMBR(FILE* dst, const char* path)
{
    append(dst, path);
}

int main(int argc, char** argv)
{
    FILE* f;

    if (argc != 3)
        return 1;

    f = fopen(argv[1], "wb");
    makeMBR(f, argv[2]);
    padCHS(f);
    fclose(f);
    return 0;
}
