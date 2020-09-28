#ifndef ARCH_X86_PAGES_H
#define ARCH_X86_PAGES_H 1

#define X86_PAGE_PRESENT      0x01
#define X86_PAGE_WRITE        0x02
#define X86_PAGE_USER         0x04
#define X86_PAGE_WRITETHROUGH 0x08
#define X86_PAGE_NOCACHE      0x10
#define X86_PAGE_ACCESSED     0x20
#define X86_PAGE_LARGE        0x80
#define X86_PAGE_GLOBAL       0x100
#define X86_PAGE_NOEXEC       0x8000000000000000

#endif
