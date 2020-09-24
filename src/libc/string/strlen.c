#include <string.h>

size_t strlen(const char* str)
{
    size_t i;

    i = 0;
    for (;;)
    {
        if (str[i] == 0)
            return i;
        i++;
    }
}
