#ifndef BOOT_H
#define BOOT_H 1

#include <boot/bootloader/vbe.h>
#include <libboot/libboot.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>

#define PAGESIZE 4096

typedef struct
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;

    uint16_t es;
    uint16_t fs;
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
void memory_identity_map(void);

/* Disk */
void disk_read_raw(char* dst, uint64_t lba, uint32_t sectors);

/* MFS */
void     mfs_init(void);
uint64_t mfs_lookup_at(uint64_t inode, const char* name);
uint64_t mfs_lookup_root(const char* name);
uint64_t mfs_file_size(uint64_t inode);
void     mfs_read(char* dst, uint64_t inode);

/* InitRAM */
void  initram_init(void);
char* initram_lookup(const char* name);

/* alloc */
uint64_t    alloc_page_lo(void);
uint64_t    alloc_page_hi(void);
void        valloc(void* addr, size_t size);

/* elf */
uint64_t elf_load(const char* name);

void jump_mode_long(uint64_t entry, uint32_t cr3, BootParams* params);

#endif
