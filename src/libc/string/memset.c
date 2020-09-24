#include <string.h>

void* memset(void* dst, int c, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        ((char*)dst)[i] = (char)c;
    return (char*)dst + size;
}
