#include <kernel/kernel.h>

static int64_t test(Process* proc, SysArg* args)
{
    return 0;
};

SysHandler gSysHandlers[KERNEL_MAX_SYSCALL] = {
    &test
};
