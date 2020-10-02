#ifndef LIBBOOT_VIDEO_H
#define LIBBOOT_VIDEO_H

#include <libboot/common.h>
#include <stdint.h>
#include <sys/_cext.h>

#define FONT_W 8
#define FONT_H 8

typedef void (*BootVideoPutcharFunc)(char, uint16_t, uint16_t);
typedef void (*BootVideoScrollFunc)(void);

typedef struct
{
    BootVideoPutcharFunc putchar;
    BootVideoScrollFunc  scroll;
} BootVideModeHandler;

typedef struct
{
    BOOT_ALIGNED(uint32_t mode);
    BOOT_ALIGNED(uint16_t width);
    BOOT_ALIGNED(uint16_t height);
    BOOT_ALIGNED(uint8_t bpp);
    BOOT_ALIGNED(uint32_t pitch);
    BOOT_ALIGNED(uint16_t text_cursor_x);
    BOOT_ALIGNED(uint16_t text_cursor_y);
    BOOT_ALIGNED(uint16_t text_max_x);
    BOOT_ALIGNED(uint16_t text_max_y);
    BOOT_ALIGNED(volatile void* framebuffer);
} BootVideo;

_EXTERNC void video_init(void);
_EXTERNC void video_set_graphical_mode(void* framebuffer, uint16_t width, uint16_t height, uint8_t bpp, uint32_t pitch);

_EXTERNC void video_putchar(char c);

_EXTERNC void video_text_putchar(char c, uint16_t x, uint16_t y);
_EXTERNC void video_text_scroll(void);

_EXTERNC void video_gfx_putchar(char c, uint16_t x, uint16_t y);
_EXTERNC void video_gfx_scroll(void);

#endif
