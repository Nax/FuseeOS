#ifndef X86_DEFS_H
#define X86_DEFS_H 1

/* Registers */
#define X86_REG_AX      0
#define X86_REG_BX      1
#define X86_REG_CX      2
#define X86_REG_DX      3
#define X86_REG_SI      4
#define X86_REG_DI      5
#define X86_REG_SP      6
#define X86_REG_BP      7
#define X86_REG_R8      8
#define X86_REG_R9      9
#define X86_REG_R10     10
#define X86_REG_R11     11
#define X86_REG_R12     12
#define X86_REG_R13     13
#define X86_REG_R14     14
#define X86_REG_R15     15
#define X86_REG_IP      16
#define X86_REG_FLAGS   17

/* Flags */
#define X86_FLAG_CF  0x00000001
#define X86_FLAG_PF  0x00000004
#define X86_FLAG_AF  0x00000010
#define X86_FLAG_ZF  0x00000040
#define X86_FLAG_SF  0x00000080
#define X86_FLAG_TF  0x00000100
#define X86_FLAG_IF  0x00000200
#define X86_FLAG_DF  0x00000400
#define X86_FLAG_OF  0x00000800
#define X86_FLAG_NT  0x00004000
#define X86_FLAG_RT  0x00010000
#define X86_FLAG_VM  0x00020000
#define X86_FLAG_AC  0x00040000
#define X86_FLAG_VIF 0x00080000
#define X86_FLAG_VIP 0x00100000
#define X86_FLAG_ID  0x00200000

/* Traps */
#define X86_TRAP_DE  0x00
#define X86_TRAP_DB  0x01
#define X86_TRAP_NMI 0x02
#define X86_TRAP_BP  0x03
#define X86_TRAP_OF  0x04
#define X86_TRAP_BR  0x05
#define X86_TRAP_UD  0x06
#define X86_TRAP_NM  0x07
#define X86_TRAP_DF  0x08
#define X86_TRAP_TS  0x0a
#define X86_TRAP_NP  0x0b
#define X86_TRAP_SS  0x0c
#define X86_TRAP_GP  0x0d
#define X86_TRAP_PF  0x0e
#define X86_TRAP_MF  0x10
#define X86_TRAP_AC  0x11
#define X86_TRAP_MC  0x12

/* Interrupts */
#define X86_INT_OFFSET_PIC 0x20

#define X86_INT_TIMER    X86_INT_OFFSET_PIC + X86_PIC_TIMER
#define X86_INT_KEYBOARD X86_INT_OFFSET_PIC + X86_PIC_KEYBOARD
#define X86_INT_COM2     X86_INT_OFFSET_PIC + X86_PIC_COM2
#define X86_INT_COM1     X86_INT_OFFSET_PIC + X86_PIC_COM1
#define X86_INT_LPT2     X86_INT_OFFSET_PIC + X86_PIC_LPT2
#define X86_INT_FLOPPY   X86_INT_OFFSET_PIC + X86_PIC_FLOPPY
#define X86_INT_LPT1     X86_INT_OFFSET_PIC + X86_PIC_LPT1
#define X86_INT_RTC      X86_INT_OFFSET_PIC + X86_PIC_RTC
#define X86_INT_MOUSE    X86_INT_OFFSET_PIC + X86_PIC_MOUSE
#define X86_INT_FPU      X86_INT_OFFSET_PIC + X86_PIC_FPU
#define X86_INT_ATA1     X86_INT_OFFSET_PIC + X86_PIC_ATA1
#define X86_INT_ATA2     X86_INT_OFFSET_PIC + X86_PIC_ATA2

/* MSRs */
#define X86_MSR_EFER            0xc0000080
#define X86_MSR_STAR            0xc0000081
#define X86_MSR_LSTAR           0xc0000082
#define X86_MSR_CSTAR           0xc0000083
#define X86_MSR_SFMASK          0xc0000084
#define X86_MSR_FS_BASE         0xc0000100
#define X86_MSR_GS_BASE         0xc0000101
#define X86_MSR_KERNEL_GS_BASE  0xc0000102

/* Page flags */
#define X86_PAGE_PRESENT      0x01
#define X86_PAGE_WRITE        0x02
#define X86_PAGE_USER         0x04
#define X86_PAGE_WRITETHROUGH 0x08
#define X86_PAGE_NOCACHE      0x10
#define X86_PAGE_ACCESSED     0x20
#define X86_PAGE_LARGE        0x80
#define X86_PAGE_GLOBAL       0x100
#define X86_PAGE_NOEXEC       0x8000000000000000

/* Selectors */
#define X86_SEL_NULL    0x00
#define X86_SEL_CODE0   0x08
#define X86_SEL_DATA0   0x10
#define X86_SEL_CODE3   0x18
#define X86_SEL_DATA3   0x20
#define X86_SEL_TSS     0x30

/* Rings */
#define X86_RING0 0
#define X86_RING1 1
#define X86_RING2 2
#define X86_RING3 3

/* IO */
#define X86_IO_PIC1_COMMAND 0x0020
#define X86_IO_PIC1_DATA    0x0021
#define X86_IO_PIC2_COMMAND 0x00a0
#define X86_IO_PIC2_DATA    0x00a1
#define X86_IO_TIMER_DATA0  0x0040
#define X86_IO_TIMER_DATA1  0x0041
#define X86_IO_TIMER_DATA2  0x0042
#define X86_IO_TIMER_CMD    0x0043


/* PIC */
#define X86_PIC_TIMER    0x00
#define X86_PIC_KEYBOARD 0x01
#define X86_PIC_COM2     0x03
#define X86_PIC_COM1     0x04
#define X86_PIC_LPT2     0x05
#define X86_PIC_FLOPPY   0x06
#define X86_PIC_LPT1     0x07
#define X86_PIC_RTC      0x08
#define X86_PIC_MOUSE    0x0c
#define X86_PIC_FPU      0x0d
#define X86_PIC_ATA1     0x0e
#define X86_PIC_ATA2     0x0f

#endif
