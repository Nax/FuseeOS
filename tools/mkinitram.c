#include <ctype.h>
#include <fusee/initram.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFFERSIZE 4096

typedef struct
{
    InitRamHeader     header;
    InitRamFileEntry* entries;
    char*             names;
    size_t            names_size;
    char*             data;

} InitRamDisk;

static void init_initram(InitRamDisk* rd)
{
    memset(rd, 0, sizeof(*rd));
    memcpy(rd->header.magic, INITRAM_MAGIC, 8);

    /* TODO: dynamically grow this */
    rd->entries = calloc(256, sizeof(InitRamFileEntry));
    rd->names   = calloc(1, 4096);
}

static void save_initram(InitRamDisk* rd, const char* path)
{
    FILE*    f;
    uint64_t off;

    off               = sizeof(InitRamHeader);
    rd->header.sh_off = off;
    off += rd->header.sh_size;
    rd->header.fh_off = off;
    off += rd->header.fh_size;
    rd->header.data_off = off;

    f = fopen(path, "wb");
    fwrite(&rd->header, sizeof(InitRamHeader), 1, f);
    fwrite(rd->names, rd->header.sh_size, 1, f);
    fwrite((char*)rd->entries, rd->header.fh_size, 1, f);
    fwrite(rd->data, rd->header.data_size, 1, f);
    fclose(f);
}

static void add_file(InitRamDisk* rd, const char* name, const char* path)
{
    FILE*  f;
    int    file_index;
    size_t namelen;
    size_t file_size;

    /* Handle name */
    namelen = strlen(name);

    rd->entries[rd->header.fh_cnt].name_off  = rd->header.sh_size;
    rd->entries[rd->header.fh_cnt].name_size = namelen;
    memcpy(rd->names + rd->header.sh_size, name, namelen + 1);
    rd->header.sh_size += namelen + 1;

    /* Handle data */
    f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    rd->entries[rd->header.fh_cnt].data_off  = rd->header.data_size;
    rd->entries[rd->header.fh_cnt].data_size = file_size;
    rd->data                                 = realloc(rd->data, rd->header.data_size + file_size);
    fread(rd->data + rd->header.data_size, file_size, 1, f);
    fclose(f);
    rd->header.data_size += file_size;

    /* Handle file entries */
    rd->header.fh_cnt++;
    rd->header.fh_size += sizeof(InitRamFileEntry);
}

static void parse_initram_str(char* buf, FILE* f)
{
    int c;
    int cursor = 0;

    /* Skip leading whitespace */
    for (;;)
    {
        c = fgetc(f);
        if (!isspace(c))
            break;
    }

    buf[cursor++] = c;

    for (;;)
    {
        c = fgetc(f);
        if (isspace(c))
            break;
        buf[cursor++] = c;
    }

    buf[cursor] = 0;
}

static void parse_initram_file(InitRamDisk* rd, FILE* f)
{
    char dst[1024];
    char src[1024];

    parse_initram_str(dst, f);
    parse_initram_str(src, f);
    add_file(rd, dst, src);
}

static void parse_initram_src(InitRamDisk* rd, const char* path)
{
    FILE* f;
    int   c;

    f = fopen(path, "r");
    for (;;)
    {
        if (feof(f))
            break;
        c = fgetc(f);
        switch (c)
        {
        case '#':
            while (c != '\n' && !feof(f))
            {
                c = fgetc(f);
            }
            break;
        case 'F':
            parse_initram_file(rd, f);
            break;
        default:
            break;
        }
    }
    fclose(f);
}

int main(int argc, char** argv)
{
    InitRamDisk rd;

    init_initram(&rd);
    parse_initram_src(&rd, argv[2]);
    save_initram(&rd, argv[1]);
    return 0;
}
