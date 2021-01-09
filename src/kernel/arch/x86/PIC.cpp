#include <kernel/arch/x86/defs.h>
#include <kernel/arch/x86/asm.h>
#include <kernel/arch/x86/PIC.h>

#define ICW1_ICW4	        0x01
#define ICW1_SINGLE	        0x02
#define ICW1_INTERVAL4	    0x04
#define ICW1_LEVEL	        0x08
#define ICW1_INIT	        0x10

#define ICW4_8086	        0x01
#define ICW4_AUTO	        0x02
#define ICW4_BUF_SLAVE	    0x08
#define ICW4_BUF_MASTER	    0x0c
#define ICW4_SFNM	        0x10

void pic_init()
{
    /* Reset the PIC */
    out8(X86_IO_PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out8(X86_IO_PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    /* Send the interrupt offset */
    out8(X86_IO_PIC1_DATA, X86_INT_OFFSET_PIC);
    io_wait();
    out8(X86_IO_PIC2_DATA, X86_INT_OFFSET_PIC + 8);
    io_wait();

    /* Setup the slave/master PIC */
    out8(X86_IO_PIC1_DATA, 4);
    io_wait();
    out8(X86_IO_PIC2_DATA, 2);
    io_wait();

    /* Setup 8086 emu mode */
    out8(X86_IO_PIC1_DATA, ICW4_8086);
    io_wait();
    out8(X86_IO_PIC2_DATA, ICW4_8086);
    io_wait();

    /* Set the masks */
    out8(X86_IO_PIC1_DATA, 0x04);
    out8(X86_IO_PIC2_DATA, 0x00);
}

void pic_enable(int i)
{
    uint16_t port;
    uint8_t value;

    if (i < 8)
    {
        port = X86_IO_PIC1_DATA;
    }
    else
    {
        i -= 8;
        port = X86_IO_PIC2_DATA;
    }

    value = in8(port);
    value &= ~(1 << i);
    out8(port, value);
}

void pic_disable(int i)
{
    uint16_t port;
    uint8_t value;

    if (i < 8)
    {
        port = X86_IO_PIC1_DATA;
    }
    else
    {
        i -= 8;
        port = X86_IO_PIC2_DATA;
    }

    value = in8(port);
    value |= (1 << i);
    out8(port, value);
}
