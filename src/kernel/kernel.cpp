#include <kernel/kernel.h>

/**
 * This function is called by most interrupt handlers when they are done.
 * It's main function is to find something to do and to return to userland.
 */
void kern_schedule(void)
{
    exec_proc(gKernel.procs.next());
}
