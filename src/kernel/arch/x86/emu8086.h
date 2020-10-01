#ifndef ARCH_X86_EMU8086_H
#define ARCH_X86_EMU8086_H 1

#include <stddef.h>
#include <stdint.h>
#include <sys/_cext.h>

#define X86_EMU_EAX    0
#define X86_EMU_EBX    3
#define X86_EMU_ECX    1
#define X86_EMU_EDX    2
#define X86_EMU_ESI    6
#define X86_EMU_EDI    7
#define X86_EMU_ESP    4
#define X86_EMU_EBP    5
#define X86_EMU_EIP    8
#define X86_EMU_EFLAGS 9

#define X86_EMU_CS 1
#define X86_EMU_DS 3
#define X86_EMU_ES 0
#define X86_EMU_FS 4
#define X86_EMU_GS 5
#define X86_EMU_SS 2

#define X86_EMU_NULLRET 0x0000

union Emu8086Reg
{
    uint8_t  u8;
    uint16_t u16;
    uint32_t u32;
    int8_t   i8;
    int16_t  i16;
    int32_t  i32;
};

struct Emu8086
{
    Emu8086Reg regs[10];
    uint16_t   sregs[6];

    volatile uint8_t* bios;
    uint8_t           ivt[0x400];
    uint8_t           bda[0x100];
    uint8_t           stack[0x4000];
    uint8_t           ram[0x4000];
    uint8_t           ebda[1024 * 128];
};

struct Emu8086BiosArgs
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;

    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
};

struct Emu8086ModRM
{
    uint8_t rm : 3;
    uint8_t reg : 3;
    uint8_t mod : 2;
    union
    {
        int8_t  disp8;
        int16_t disp16;
        int32_t disp32;
    };

    int8_t seg_override;
};

void emu8086_init();
int  emu8086_bios_int(int intnum, Emu8086BiosArgs* args);
void emu8086_write(uint16_t seg, uint16_t base, void* addr, size_t size);
void emu8086_read(void* addr, uint16_t seg, uint16_t base, size_t size);

#endif
