#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

#include <stdint.h>
#include <sys/_cext.h>

#define SYS_IRQ_WAIT    0

#if (!defined(__LIBC_MINIMAL__))
#define __syscall0(sys)                     __syscall6(sys, 0, 0, 0, 0, 0, 0)
#define __syscall1(sys, a1)                 __syscall6(sys, a1, 0, 0, 0, 0, 0)
#define __syscall2(sys, a1, a2)             __syscall6(sys, a1, a2, 0, 0, 0, 0)
#define __syscall3(sys, a1, a2, a3)         __syscall6(sys, a1, a2, a3, 0, 0, 0)
#define __syscall4(sys, a1, a2, a3, a4)     __syscall6(sys, a1, a2, a3, a4, 0, 0)
#define __syscall5(sys, a1, a2, a3, a4, a5) __syscall6(sys, a1, a2, a3, a4, a5, 0)
_EXTERNC int64_t __syscall6(int sys, int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t a6);
#endif

#endif
