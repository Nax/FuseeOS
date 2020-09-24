#include <string.h>

int strcmp(const char* s1, const char* s2)
{
    size_t i;
    int    c1;
    int    c2;
    int    diff;

    i = 0;
    for (;;)
    {
        c1   = ((int)(((unsigned char*)s1)[i]));
        c2   = ((int)(((unsigned char*)s2)[i]));
        diff = c2 - c1;
        if (diff)
            return diff;
        if (!c1)
            return 0;
    }
}
