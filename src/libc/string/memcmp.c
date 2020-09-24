#include <string.h>

int memcmp(const void* s1, const void* s2, size_t n)
{
    int diff;

    for (size_t i = 0; i < n; ++i)
    {
        diff = ((int)(((unsigned char*)s2)[i])) - ((int)(((unsigned char*)s1)[i]));
        if (diff)
            return diff;
    }
    return 0;
}
