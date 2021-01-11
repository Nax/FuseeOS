#include <kernel/arch.h>

_EXTERNC void x86_sys_handler();

void x86_sys_init()
{
    wrmsr(X86_MSR_SFMASK,   X86_FLAG_IF);
    wrmsr(X86_MSR_LSTAR,    (uint64_t)&x86_sys_handler);
    wrmsr(X86_MSR_STAR,     ((uint64_t)X86_SEL_CODE0 << 32) | ((uint64_t)X86_SEL_CODE3 << 48));
}
