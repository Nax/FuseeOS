#ifndef ARCH_X86_TSS_H
#define ARCH_X86_TSS_H

#include <cstdint>
#include <sys/_cext.h>

struct _PACKED TSS
{
    std::uint32_t   reserved0;
    std::uint64_t   rsp[3];
    std::uint64_t   reserved1;
    std::uint64_t   ist[7];
    char            reserved2[10];
    std::uint16_t   iopb_off;
};

#endif
