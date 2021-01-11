#include <sys/syscall.h>
#include <sys/irq.h>

#if !defined(__LIBC_MINIMAL__)

int irq_wait(int mask)
{
    return __syscall1(SYS_IRQ_WAIT, mask);
}

#endif
