#include <kernel/kernel.h>

SysHandler gSysHandlers[KERNEL_MAX_SYSCALL] = {
    nullptr
};
