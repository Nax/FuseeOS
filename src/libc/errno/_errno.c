#include <errno.h>

static int e;

int* _errno(void)
{
    return &e;
}
