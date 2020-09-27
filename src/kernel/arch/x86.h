#ifndef KERNEL_ARCH_X86_H
#define KERNEL_ARCH_x86_H 1

#include <stdint.h>

#define MSR_IA32_EFER 0xc0000080

inline static void cpuid2(uint64_t* dst, uint64_t fun, uint64_t fun2)
{
    __asm__ __volatile__(
        "cpuid\r\n"
        : "=a"(dst[0]), "=b"(dst[1]), "=c"(dst[2]), "=d"(dst[3])
        : "a"(fun), "c"(fun2));
}

inline static void cpuid(uint64_t* dst, uint64_t fun)
{
    cpuid2(dst, fun, 0);
}

inline static uint64_t rdmsr(uint32_t msr)
{
    uint32_t hi;
    uint32_t lo;

    __asm__ __volatile__(
        "rdmsr\r\n"
        : "=d"(hi), "=a"(lo)
        : "c"(msr));

    return (((uint64_t)hi) << 32) | lo;
}

inline static void wrmsr(uint32_t msr, uint64_t value)
{
    uint32_t hi;
    uint32_t lo;

    hi = value >> 32;
    lo = value & 0xffffffff;

    __asm__ __volatile__("wrmsr\r\n" ::"c"(msr), "a"(lo), "d"(hi));
}

#endif
