#include <kernel/kernel.h>

void init_mem(void)
{
    init_physical_mapping();
    init_physical_memory();
    init_virtual_memory();
    kmprotect_kernel();
}
