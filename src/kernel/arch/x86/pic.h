#ifndef ARCH_X86_PIC_H
#define ARCH_X86_PIC_H

#include <sys/_cext.h>

_EXTERNC void pic_init(void);
_EXTERNC void pic_enable(int i);
_EXTERNC void pic_disable(int i);

#endif
