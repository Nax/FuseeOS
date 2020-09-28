#ifndef ARCH_X86_ASM_H
#define ARCH_X86_ASM_H 1

#include <stdint.h>

#define ASM __asm__ __volatile__

inline static void cpuid2(uint64_t* dst, uint64_t fun, uint64_t fun2)
{
    ASM(
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

    ASM(
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

    ASM("wrmsr\r\n" ::"c"(msr), "a"(lo), "d"(hi));
}

#endif
