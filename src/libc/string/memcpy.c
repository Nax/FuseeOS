#include <string.h>

void* memcpy(void* _RESTRICT dst, const void* _RESTRICT src, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        ((char*)dst)[i] = ((char*)src)[i];
    return (char*)dst + size;
}
