#ifndef _ERRNO_H
#define _ERRNO_H 1

#include <sys/_errno.h>

int* _errno(void);
#define errno (*(_errno()))

#endif
