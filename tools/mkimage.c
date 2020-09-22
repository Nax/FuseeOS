#include <stdint.h>
#include <stdio.h>

#define BUFFERSIZE  4096
#define MAX_SECTORS 63
#define MAX_HEADS   256

static void pad(FILE* f, size_t amount)
{
    char   zero;
    size_t n;

    n = ftell(f) % amount;
    if (!n)
        return;
    fseek(f, amount - n - 1, SEEK_CUR);
    zero = 0;
    fwrite(&zero, 1, 1, f);
}

static size_t append(FILE* dst, const char* path)
{
    FILE*  src;
    char   buffer[BUFFERSIZE];
    size_t size;
    size_t total_size;

    total_size = 0;
    src        = fopen(path, "rb");
    for (;;)
    {
        size = fread(buffer, 1, BUFFERSIZE, src);
        total_size += size;
        if (!size)
            break;
        fwrite(buffer, size, 1, dst);
    }
    fclose(src);
    return total_size;
}

static void makeMBR(FILE* dst, const char* path)
{
    append(dst, path);
    pad(dst, 512 * 64);
}

static void to_chs(size_t lba, int* cyl, int* head, int* sec)
{
    *sec  = ((lba / 512) % 63) + 1;
    *head = (lba / (512 * 63)) % 128;
    *cyl  = lba / (512 * 63 * 128);
}

static void add_volume(FILE* dst, const char* path)
{
    size_t base;
    size_t size;
    size_t tmp;

    int cyl;
    int head;
    int sec;

    char record[16];

    base = ftell(dst);
    size = append(dst, path);
    pad(dst, 512);
    tmp = ftell(dst);
    fseek(dst, 0x01be, SEEK_SET);

    record[0] = 0x80;
    to_chs(base, &cyl, &head, &sec);
    record[1] = (char)head;
    record[2] = (sec & 0x3f) | ((cyl >> 2) & 0xc0);
    record[3] = (char)cyl;
    record[4] = 0x60;
    to_chs(base + size - 512, &cyl, &head, &sec);
    record[5]  = (char)head;
    record[6]  = (sec & 0x3f) | ((cyl >> 2) & 0xc0);
    record[7]  = (char)cyl;
    record[8]  = (base & 0xff);
    record[9]  = ((base >> 8) & 0xff);
    record[10] = ((base >> 16) & 0xff);
    record[11] = ((base >> 24) & 0xff);
    record[12] = (size & 0xff);
    record[13] = ((size >> 8) & 0xff);
    record[14] = ((size >> 16) & 0xff);
    record[15] = ((size >> 24) & 0xff);
    fwrite(record, 16, 1, dst);
    fseek(dst, tmp, SEEK_SET);
}

int main(int argc, char** argv)
{
    FILE* f;

    if (argc != 4)
        return 1;

    f = fopen(argv[1], "wb");
    makeMBR(f, argv[2]);
    add_volume(f, argv[3]);
    pad(f, 512 * MAX_SECTORS * MAX_HEADS);
    fclose(f);
    return 0;
}
