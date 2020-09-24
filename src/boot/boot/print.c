#include "boot.h"

#define SCREEN_BUFFER ((short*)0xb8000)
#define MAX_X         80
#define MAX_Y         25

int cx;
int cy;

void screen_init(void)
{
    BiosArgs args;

    /* Set video mode 0x03 */
    args.eax = 0x0003;
    bios_call(0x10, &args);

    /* Hide cursor */
    args.eax = 0x0100;
    args.ecx = 0x2607;
    bios_call(0x10, &args);

    /* Clear the screen buffer */
    bzero(SCREEN_BUFFER, MAX_X * MAX_Y * 2);

    /* Reset cursor pos */
    cx = 0;
    cy = 0;
}

void putchar(int c)
{
    if (cx == MAX_X)
    {
        cx = 0;
        cy++;
    }

    if (cy == MAX_Y)
    {
        cy--;
        memmove(SCREEN_BUFFER, SCREEN_BUFFER + MAX_X, MAX_X * (MAX_Y - 1) * 2);
        bzero(SCREEN_BUFFER + MAX_X * (MAX_Y - 1) * 2, MAX_X * 2);
    }

    switch (c)
    {
    case '\n':
        cx = 0;
        cy++;
        break;
    default:
        SCREEN_BUFFER[cy * MAX_X + cx] = (0x0f00 | (c & 0xff));
        cx++;
        break;
    }
}

void print(const char* str)
{
    int i = 0;
    int c;

    for (;;)
    {
        c = str[i++];
        if (!c)
            break;
        putchar(c);
    }
}

void puts(const char* str)
{
    print(str);
    putchar('\n');
}
