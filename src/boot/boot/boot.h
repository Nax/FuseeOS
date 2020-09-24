#ifndef BOOT_H
#define BOOT_H 1

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/kernel.h>

extern KernelBootParams g_kernel_params;

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
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t attr;
} _PACKED BiosMemRegion;

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

void puthex8(uint8_t);
void puthex16(uint16_t);
void puthex32(uint32_t);
void puthex64(uint64_t);

/* Memory */
void memory_detect(void);

#endif
