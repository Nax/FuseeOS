#include <kernel/arch.h>
#include <kernel/kernel.h>

static void init_nx(void)
{
    uint64_t info[4];
    uint64_t tmp;

    cpuid(info, 0x80000000);
    if (info[0] < 0x80000001)
        return;
    cpuid(info, 0x80000001);
    if (info[3] & (1 << 20))
    {
        /* Set the NX flag */
        gKernel.nx_mask = 0x8000000000000000;

        /* Enable the feature in EFER */
        tmp = rdmsr(MSR_IA32_EFER);
        tmp |= (1 << 11);
        wrmsr(MSR_IA32_EFER, tmp);
    }
}

void init_mem(void)
{
    /* Initialize the memory subsystems */
    init_physical_mapping();
    init_physical_memory();
    init_virtual_memory();

    /* Detect availability of the NX flag */
    init_nx();

    kmprotect_kernel();
}
