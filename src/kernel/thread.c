#include <kernel/kernel.h>

void thread_init()
{
    gKernel.threads = (KernelThread*)kmalloc(sizeof(KernelThread), 0);
    memset(gKernel.threads, 0, sizeof(KernelThread));
    gKernel.threads[0].self = &gKernel.threads[0];
    gKernel.threads[0].arch.tss.rsp[0] = (uint64_t)gKernel.threads[0].stack + sizeof(gKernel.threads[0].stack);
    gKernel.threads[0].arch.tss.iopb_off = sizeof(TSS);
    gdt_add_tss(X86_SEL_TSS, (uint64_t)&gKernel.threads[0].arch.tss);
    gdt_reload();
    ASM("ltr %w0\r\n" :: "r"(X86_SEL_TSS));

    /* Set up kernel TLS */
    wrmsr(X86_MSR_FS_BASE, 0);
    wrmsr(X86_MSR_GS_BASE, (uint64_t)gKernel.threads);
    wrmsr(X86_MSR_KERNEL_GS_BASE, 0);
}
