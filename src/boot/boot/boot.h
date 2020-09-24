#ifndef BOOH_H
#define BOOT_H 1

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/kernel.h>

typedef struct
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
} _PACKED BiosArgs;

typedef struct
{
    uint8_t  size;
    uint8_t  zero;
    uint16_t transfer_size;
    uint16_t dst_offset;
    uint16_t dst_segment;
    uint32_t lba_lo;
    uint32_t lba_hi;
} _PACKED DiskAccessPacket;

int bios_call(int num, BiosArgs* args);

/* Screen */
void screen_init(void);
void putchar(int c);
void print(const char* str);
void puts(const char* str);

#endif
