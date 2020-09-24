#include <string.h>
#include <strings.h>

void bzero(void* dst, size_t size)
{
    memset(dst, 0, size);
}
