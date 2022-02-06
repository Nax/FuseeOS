#ifndef ARCH_X86_TSS_H
#define ARCH_X86_TSS_H

#include <stdint.h>
#include <sys/_cext.h>

typedef struct _PACKED TSS
{
    uint32_t    reserved0;
    uint64_t    rsp[3];
    uint64_t    reserved1;
    uint64_t    ist[7];
    char        reserved2[10];
    uint16_t    iopb_off;
} TSS;

#endif
