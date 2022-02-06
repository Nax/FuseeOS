#include <kernel/kernel.h>
#include <kernel/arch/x86/pic.h>

void timer_ns(uint64_t ns)
{
    uint16_t divider;

    divider = ns * 1000000000 / 1193182;
    out8(X86_IO_TIMER_CMD, 0x34);
    io_wait();
    out8(X86_IO_TIMER_DATA0, divider & 0xff);
    io_wait();
    out8(X86_IO_TIMER_DATA0, (divider >> 8));
    pic_enable(X86_PIC_TIMER);
}
