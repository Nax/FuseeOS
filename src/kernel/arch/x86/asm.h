#ifndef ARCH_X86_ASM_H
#define ARCH_X86_ASM_H 1

#include <stdint.h>

#define ASM __asm__ __volatile__

inline static void cpuid2(uint64_t* dst, uint64_t fun, uint64_t fun2)
{
    ASM("cpuid\r\n" : "=a"(dst[0]), "=b"(dst[1]), "=c"(dst[2]), "=d"(dst[3]) : "a"(fun), "c"(fun2));
}

inline static void cpuid(uint64_t* dst, uint64_t fun) { cpuid2(dst, fun, 0); }

inline static uint64_t rdmsr(uint32_t msr)
{
    uint32_t hi;
    uint32_t lo;

    ASM("rdmsr\r\n" : "=d"(hi), "=a"(lo) : "c"(msr));

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

inline static uint8_t in8(uint16_t port)
{
    uint8_t v;

    ASM("inb %1, %0\r\n" : "=a"(v) : "Nd"(port));
    return v;
}

inline static uint16_t in16(uint16_t port)
{
    uint16_t v;

    ASM("inw %1, %0\r\n" : "=a"(v) : "Nd"(port));
    return v;
}

inline static uint32_t in32(uint16_t port)
{
    uint32_t v;

    ASM("inl %1, %0\r\n" : "=a"(v) : "Nd"(port));
    return v;
}

inline static void out8(uint16_t port, uint8_t value) { ASM("outb %0, %1\r\n" ::"a"(value), "Nd"(port)); }

inline static void out16(uint16_t port, uint16_t value) { ASM("outw %0, %1\r\n" ::"a"(value), "Nd"(port)); }

inline static void out32(uint16_t port, uint32_t value) { ASM("outl %0, %1\r\n" ::"a"(value), "Nd"(port)); }

inline static void io_wait(void) { out8(0x80, 0x00); }

inline static uint64_t getflags()
{
    uint64_t tmp;

    ASM(
        "pushf\r\n"
        "popq %0\r\n"
        : "=r"(tmp)
    );

    return tmp;
}

#endif
