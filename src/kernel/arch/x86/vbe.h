#ifndef ARCH_X86_VBE_H
#define ARCH_X86_VBE_H

#include <stdint.h>
#include <sys/_cext.h>

struct VbeInfoBlock
{
    char     magic[4];
    uint16_t version;
    uint16_t oem_ptr[2];
    uint8_t  caps[4];
    uint16_t video_modes_ptr[2];
    uint16_t total_memory;
} _PACKED;

struct VbeVideoMode
{
    uint16_t attr;
    uint8_t  win_a;
    uint8_t  win_b;
    uint16_t granularity;
    uint16_t winsize;
    uint16_t segmentA;
    uint16_t segmentB;
    uint16_t real_func_ptr[2];
    uint16_t pitch;

    uint16_t xres;
    uint16_t yres;
    uint8_t  Wchar, Ychar, planes, bpp, banks;
    uint8_t  memory_model, bank_size, image_pages;
    uint8_t  reserved0;

    uint8_t red_mask, red_position;
    uint8_t green_mask, green_position;
    uint8_t blue_mask, blue_position;
    uint8_t rsv_mask, rsv_position;
    uint8_t directcolor_attributes;

    uint32_t physbase;
    uint32_t reserved1;
    uint16_t reserved2;
} _PACKED;

void vbe_init(void);

#endif
