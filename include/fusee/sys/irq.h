#ifndef _SYS_IRQ_H
#define _SYS_IRQ_H 1

#include <sys/_cext.h>

/**
 * Blocks the current process until an IRQ is triggered.
 * Args:
 *   mask - A bitmap of IRQs to wait for
 * Returns:
 *   n >= 0: IRQ #n was triggered
 *   n < 0 : An error occured
 */
_EXTERNC int irq_wait(int mask);

#endif
