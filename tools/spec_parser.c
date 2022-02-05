#include "spec_parser.h"
#include <stdlib.h>

static int parse_spec_str(char* buf, FILE* f)
{
    int c;
    int cursor = 0;

    for (;;)
    {
        c = fgetc(f);
        if (c == '\n' || c == '\r' || c == ',' || c == EOF)
            break;
        buf[cursor++] = c;
    }

    buf[cursor] = 0;
    return 0;
}

static int parse_spec_line(Spec* spec, FILE* f)
{
    parse_spec_str(spec->dst, f);
    parse_spec_str(spec->src, f);
    return 0;
}

int parse_spec(Spec* spec, FILE* f)
{
    int c;

    for (;;)
    {
        if (feof(f))
            return 1;
        for (;;)
        {
            c = fgetc(f);
            if (c != '\n' && c != '\r')
                break;
        }

        switch (c)
        {
        case '#':
            while (c != '\n' && !feof(f))
            {
                c = fgetc(f);
            }
            break;
        default:
            spec->type = c;
            fgetc(f); // Skip ,
            return parse_spec_line(spec, f);
        }
    }
}
