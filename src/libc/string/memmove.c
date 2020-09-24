#include <string.h>

void* memmove(void* dst, const void* src, size_t size)
{
    if (dst < src || ((char*)dst >= (char*)src + size))
    {
        for (size_t i = 0; i < size; ++i)
            ((char*)dst)[i] = ((char*)src)[i];
    }
    else
    {
        for (size_t i = 0; i < size; ++i)
            ((char*)dst)[size - i - 1] = ((char*)src)[size - i - 1];
    }
    return (char*)dst + size;
}
