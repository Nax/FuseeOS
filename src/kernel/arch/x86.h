#ifndef KERNEL_ARCH_X86_H
#define KERNEL_ARCH_x86_H 1

#include <stdint.h>

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

#endif
