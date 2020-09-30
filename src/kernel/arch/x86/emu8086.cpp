#include <type_traits>

#include <kernel/kernel.h>

#define NOT_IMPLEMENTED(x)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        print(__FILE__);                                                                                               \
        putchar(':');                                                                                                  \
        putu(__LINE__);                                                                                                \
        print(" (");                                                                                                   \
        puthex16(x);                                                                                                   \
        print(")\n");                                                                                                  \
        for (;;)                                                                                                       \
        {                                                                                                              \
            __asm__ __volatile__("mov %%rax, %%rax\r\nxchg %%bx, %%bx\r\n" ::"a"(x));                                  \
            __asm__ __volatile__("hlt\r\n");                                                                           \
        }                                                                                                              \
    } while (0)

Emu8086 gEmu;

static uint32_t seg_addr(uint32_t base, uint16_t seg) { return ((base & 0xffff) + seg * 16) & 0xfffff; }

static uint8_t read_byte(uint32_t addr)
{
    addr &= 0xfffff;

    if (addr < 0x400) return gEmu.ivt[addr];
    if (addr >= 0x400 && addr < 0x500) return gEmu.bda[addr - 0x400];
    if (addr >= 0x8000 && addr < 0x9000) return gEmu.stack[addr - 0x8000];
    if (addr >= 0xc000 && addr < 0x10000) return gEmu.ram[addr - 0xc000];
    if (addr >= 0x80000 && addr < 0xa0000) return gEmu.ebda[addr - 0x80000];
    if (addr >= 0xa0000) return gEmu.bios[addr - 0xa0000];
    return 0;
}

template <typename T> static T read(uint32_t addr)
{
    uint8_t tmp[sizeof(T)];
    T       value;

    for (size_t i = 0; i < sizeof(T); ++i)
    {
        tmp[i] = read_byte(addr + i);
    }
    memcpy(&value, tmp, sizeof(T));

    return value;
}

static void write_byte(uint32_t addr, uint8_t value)
{
    addr &= 0xfffff;

    if (addr < 0x400)
        gEmu.ivt[addr] = value;
    else if (addr >= 0x400 && addr < 0x500)
        gEmu.bda[addr - 0x400] = value;
    else if (addr >= 0x8000 && addr < 0x9000)
        gEmu.stack[addr - 0x8000] = value;
    else if (addr >= 0xc000 && addr < 0x10000)
        gEmu.ram[addr - 0xc000] = value;
    else if (addr >= 0x80000 && addr < 0xa0000)
        gEmu.ebda[addr - 0x80000] = value;
    else if (addr >= 0xa0000)
        gEmu.bios[addr - 0xa0000] = value;
}

template <typename T> static void write(uint32_t addr, T value)
{
    uint8_t tmp[sizeof(T)];

    memcpy(tmp, &value, sizeof(T));
    for (size_t i = 0; i < sizeof(T); ++i)
    {
        write_byte(addr + i, tmp[i]);
    }
}

template <typename T> static void push(T value)
{
    gEmu.regs[X86_EMU_ESP].u32 -= sizeof(T);
    write<T>(seg_addr(gEmu.regs[X86_EMU_ESP].u32, gEmu.sregs[X86_EMU_SS]), value);
}

template <typename T> static T pop(void)
{
    T tmp;

    tmp = read<T>(seg_addr(gEmu.regs[X86_EMU_ESP].u32, gEmu.sregs[X86_EMU_SS]));
    gEmu.regs[X86_EMU_ESP].u32 += sizeof(T);

    return tmp;
}

template <typename T> static void read_ip(T* ptr)
{
    *ptr = read<T>(seg_addr(gEmu.regs[X86_EMU_EIP].u32, gEmu.sregs[X86_EMU_CS]));
    gEmu.regs[X86_EMU_EIP].u16 += sizeof(T);
}

static void read_ipmodrm16(Emu8086ModRM* ptr)
{
    read_ip<uint8_t>((uint8_t*)ptr);
    switch (ptr->mod)
    {
    case 0:
        /* Special case */
        if (ptr->rm == 6) { read_ip<uint16_t>((uint16_t*)&ptr->disp16); }
        break;
    case 1:
        read_ip<uint8_t>((uint8_t*)&ptr->disp8);
        break;
    case 2:
        read_ip<uint16_t>((uint16_t*)&ptr->disp16);
        break;
    case 3:
        break;
    }
}

static void read_ipmodrm32(Emu8086ModRM* ptr)
{
    read_ip<uint8_t>((uint8_t*)ptr);
    switch (ptr->mod)
    {
    case 0:
        break;
    case 1:
        read_ip<uint8_t>((uint8_t*)&ptr->disp8);
        break;
    case 2:
        read_ip<uint32_t>((uint32_t*)&ptr->disp32);
        break;
    case 3:
        break;
    }
}

template <typename atype> static void read_ipmodrm(Emu8086ModRM* ptr, int8_t seg_override)
{
    read_ipmodrm16(ptr);
    ptr->seg_override = seg_override;
}

template <> void read_ipmodrm<uint32_t>(Emu8086ModRM* ptr, int8_t seg_override)
{
    read_ipmodrm32(ptr);
    ptr->seg_override = seg_override;
}

template <typename otype> static otype read_reg(uint8_t reg) { return (otype)gEmu.regs[reg & 0x7].u32; }

template <> uint8_t read_reg<uint8_t>(uint8_t reg)
{
    uint16_t tmp;

    tmp = gEmu.regs[reg & 3].u16;
    tmp >>= ((reg & 0x4) ? 8 : 0);
    return tmp & 0xff;
}

template <typename T> static void write_reg(uint8_t reg, T value) { memcpy(&gEmu.regs[reg].u32, &value, sizeof(T)); }

template <> void write_reg<uint8_t>(uint8_t reg, uint8_t value)
{
    uint16_t tmp;
    uint16_t mask;

    tmp  = (((uint16_t)value) << 8) | value;
    mask = (reg & 4) ? 0xff00 : 0x00ff;

    gEmu.regs[reg & 3].u16 = ((tmp & mask) | (gEmu.regs[reg & 3].u16 & ~mask));
}

static uint16_t read_sreg(uint8_t reg)
{
    reg &= 0x7;

    if (reg > 5) return 0;
    return gEmu.sregs[reg];
}

static void write_sreg(uint8_t reg, uint16_t value)
{
    reg &= 0x7;

    if (reg > 5) return;
    gEmu.sregs[reg] = value;
}

static uint32_t modrm_addr8(const Emu8086ModRM* modrm)
{
    uint16_t base;
    uint16_t seg;

    /* Special case */
    if (modrm->rm == 6 && modrm->mod == 0)
    {
        base = modrm->disp16;
        seg  = gEmu.sregs[X86_EMU_DS];
    } else
    {
        switch (modrm->rm)
        {
        case 0:
            base = gEmu.regs[X86_EMU_EBX].u16 + gEmu.regs[X86_EMU_ESI].u16;
            seg  = gEmu.sregs[X86_EMU_DS];
            break;
        case 1:
            base = gEmu.regs[X86_EMU_EBX].u16 + gEmu.regs[X86_EMU_EDI].u16;
            seg  = gEmu.sregs[X86_EMU_DS];
            break;
        case 2:
            base = gEmu.regs[X86_EMU_EBP].u16 + gEmu.regs[X86_EMU_ESI].u16;
            seg  = gEmu.sregs[X86_EMU_SS];
            break;
        case 3:
            base = gEmu.regs[X86_EMU_EBP].u16 + gEmu.regs[X86_EMU_EDI].u16;
            seg  = gEmu.sregs[X86_EMU_SS];
            break;
        case 4:
            base = gEmu.regs[X86_EMU_ESI].u16;
            seg  = gEmu.sregs[X86_EMU_DS];
            break;
        case 5:
            base = gEmu.regs[X86_EMU_EDI].u16;
            seg  = gEmu.sregs[X86_EMU_DS];
            break;
        case 6:
            base = gEmu.regs[X86_EMU_EBP].u16;
            seg  = gEmu.sregs[X86_EMU_SS];
            break;
        case 7:
            base = gEmu.regs[X86_EMU_EBX].u16;
            seg  = gEmu.sregs[X86_EMU_DS];
            break;
        }

        switch (modrm->mod)
        {
        case 0:
            break;
        case 1:
            base += modrm->disp8;
            break;
        case 2:
            base += modrm->disp16;
            break;
        }
    }

    if (modrm->seg_override != -1) { seg = gEmu.sregs[modrm->seg_override]; }

    return seg_addr(base, seg);
}

static uint32_t modrm_addr16(const Emu8086ModRM* modrm) { return modrm_addr8(modrm); }

static uint32_t modrm_addr32(const Emu8086ModRM* modrm)
{
    uint32_t base;
    uint16_t seg;

    switch (modrm->mod)
    {
    default:
        NOT_IMPLEMENTED(modrm->mod);
        break;
    }

    return seg_addr(base, seg);
}

template <typename atype> static uint32_t modrm_addr(const Emu8086ModRM* modrm) { return modrm_addr16(modrm); }

template <> uint32_t modrm_addr<uint32_t>(const Emu8086ModRM* modrm) { return modrm_addr32(modrm); }

template <typename otype, typename atype> static otype read_modrm(const Emu8086ModRM* modrm)
{
    switch (modrm->mod)
    {
    case 0x3:
        return read_reg<otype>(modrm->rm);
        break;
    default:
        return read<otype>(modrm_addr<atype>(modrm));
        break;
    }
}

template <typename otype, typename atype> static void write_modrm(const Emu8086ModRM* modrm, otype value)
{
    switch (modrm->mod)
    {
    case 0x3:
        write_reg<otype>(modrm->rm, value);
        break;
    default:
        write<otype>(modrm_addr<atype>(modrm), value);
        break;
    }
}

static bool parity(uint8_t v)
{
    v = (v & 0xf) ^ (v >> 4);
    v = (v & 0x3) ^ (v >> 2);
    v = (v & 0x1) ^ (v >> 1);

    return v == 0;
}

template <typename T> static T op_adc(T a, T b)
{
    static constexpr const uint32_t hi_bit = (1 << ((sizeof(T) * 8) - 1));

    int carryIn;
    T   res;
    T   carries;
    T   overflow;

    carryIn = (gEmu.regs[X86_EMU_EFLAGS].u32 & X86_FLAG_CF) ? 1 : 0;
    gEmu.regs[X86_EMU_EFLAGS].u32 &=
        ~(X86_FLAG_ZF | X86_FLAG_CF | X86_FLAG_OF | X86_FLAG_PF | X86_FLAG_AF | X86_FLAG_SF);

    res      = a + b + carryIn;
    carries  = a ^ b ^ res;
    overflow = (a ^ res) & (b ^ res);

    if (res == 0) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_ZF;
    if (res & hi_bit) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_SF;
    if ((carries ^ overflow) & hi_bit) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_CF;
    if (overflow & hi_bit) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_OF;
    if (parity((uint8_t)res)) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_PF;

    return res;
}

template <typename T> static T op_sbb(T a, T b)
{
    T res;

    gEmu.regs[X86_EMU_EFLAGS].u32 ^= X86_FLAG_CF;
    res = op_adc<T>(a, ~b);
    gEmu.regs[X86_EMU_EFLAGS].u32 ^= X86_FLAG_CF;

    return res;
}

template <typename T> static T op_add(T a, T b)
{
    gEmu.regs[X86_EMU_EFLAGS].u32 &= ~X86_FLAG_CF;
    return op_adc<T>(a, b);
}

template <typename T> static T op_sub(T a, T b)
{
    T value;

    gEmu.regs[X86_EMU_EFLAGS].u32 &= ~X86_FLAG_CF;
    value = op_sbb<T>(a, b);

    return value;
}

template <typename T> static T op_logic(T res)
{
    static constexpr const uint32_t hi_bit = (1 << ((sizeof(T) * 8) - 1));

    gEmu.regs[X86_EMU_EFLAGS].u32 &=
        ~(X86_FLAG_ZF | X86_FLAG_CF | X86_FLAG_OF | X86_FLAG_PF | X86_FLAG_AF | X86_FLAG_SF);

    if (res == 0) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_ZF;
    if (res & hi_bit) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_SF;
    if (parity((uint8_t)res)) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_PF;

    return res;
}

template <typename T> static T op_xor(T a, T b) { return op_logic<T>(a ^ b); }

template <typename T> static T op_and(T a, T b) { return op_logic<T>(a & b); }

template <typename T> static T op_or(T a, T b) { return op_logic<T>(a | b); }

template <typename T> static T op_shift(T original, T result, T carrymask, T ovmask)
{
    T   r;
    int carry;
    int overflow;

    r = op_logic<T>(result);

    carry    = !!(original & carrymask);
    overflow = !!(original & ovmask);

    if (carry) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_CF;

    if (carry ^ overflow) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_OF;

    return r;
}

template <typename T> static T op_shl(T a, T b)
{
    return op_shift<T>(a, a << b, 1 << (sizeof(T) * 8 - b), 1 << (sizeof(T) * 8 - 2));
}

template <typename T> static T op_shr(T a, T b) { return op_shift<T>(a, a >> b, 1 << (b - 1), 1 << b); }

template <typename T> static T op_inc(T a, int incr)
{
    static constexpr const uint32_t hi_bit = (1 << ((sizeof(T) * 8) - 1));

    T res;

    res = op_logic<T>(a + incr);
    if ((a & hi_bit) ^ (res & hi_bit)) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_OF;
    return res;
}

template <typename T> static T op_in(uint16_t port);

template <> uint8_t op_in<uint8_t>(uint16_t port) { return in8(port); }

template <> uint16_t op_in<uint16_t>(uint16_t port) { return in16(port); }

template <> uint32_t op_in<uint32_t>(uint16_t port) { return in32(port); }

template <typename T> static void op_out(uint16_t port, T value);

template <> void op_out<uint8_t>(uint16_t port, uint8_t value) { out8(port, value); }

template <> void op_out<uint16_t>(uint16_t port, uint16_t value) { out16(port, value); }

template <> void op_out<uint32_t>(uint16_t port, uint32_t value) { out32(port, value); }

template <typename T> static uint64_t op_mul_flags(uint64_t res)
{
    int64_t hi_bits;

    gEmu.regs[X86_EMU_EFLAGS].u32 &= ~(X86_FLAG_CF | X86_FLAG_OF);
    hi_bits = ((int64_t)res >> (sizeof(T) * 8 - 1));
    if (!(hi_bits == 0 || hi_bits == -1)) { gEmu.regs[X86_EMU_EFLAGS].u32 |= (X86_FLAG_CF | X86_FLAG_OF); }

    return res;
}

template <typename T> static int64_t op_imul(T a, T b)
{
    using stype = std::make_signed_t<T>;
    uint64_t res;

    res = ((int64_t)((stype)a)) * ((int64_t)((stype)b));

    return op_mul_flags<T>(res);
}

template <typename T> static int64_t op_mul(T a, T b)
{
    uint64_t res;

    res = (uint64_t)a * (uint64_t)b;

    return op_mul_flags<T>(res);
}

template <typename T> static void write_reg_ad(uint64_t value);

template <> void write_reg_ad<uint8_t>(uint64_t value) { gEmu.regs[X86_EMU_EAX].u16 = value & 0xffff; }

template <> void write_reg_ad<uint16_t>(uint64_t value)
{
    gEmu.regs[X86_EMU_EAX].u16 = value & 0xffff;
    gEmu.regs[X86_EMU_EDX].u16 = (value >> 16) & 0xffff;
}

template <> void write_reg_ad<uint32_t>(uint64_t value)
{
    gEmu.regs[X86_EMU_EAX].u32 = value & 0xffffffff;
    gEmu.regs[X86_EMU_EDX].u32 = (value >> 32) & 0xffffffff;
}

static bool test_flags(uint32_t flags) { return ((gEmu.regs[X86_EMU_EFLAGS].u32 & flags) != 0); }

static int32_t sext32(uint8_t v) { return (int32_t)((int8_t)v); }

static int32_t sext32(uint16_t v) { return (int32_t)((int16_t)v); }

static int32_t sext32(uint32_t v) { return v; }

template <typename T> void op_jump(bool cond)
{
    T imm;

    read_ip<T>(&imm);
    if (cond) gEmu.regs[X86_EMU_EIP].i32 += sext32(imm);
}

template <typename atype> static uint32_t seg_addr_ind(uint16_t reg_seg, uint16_t reg_base)
{
    return seg_addr(read_reg<atype>(reg_base), gEmu.sregs[reg_seg]);
}

template <typename otype, typename atype> static uint32_t seg_addr_ind_autoinc(uint16_t reg_seg, uint16_t reg_base)
{
    uint32_t addr;
    int      inc;

    addr = seg_addr_ind<atype>(reg_seg, reg_base);
    inc  = (gEmu.regs[X86_EMU_EFLAGS].u32 & X86_FLAG_DF) ? -(sizeof(otype)) : sizeof(otype);
    write_reg<atype>(reg_base, read_reg<atype>(reg_base) + inc);
    return addr;
}

template <typename otype, typename atype> static bool exec_instruction(uint16_t op, int8_t seg_override)
{
    uint32_t     addr_src;
    uint32_t     addr_dst;
    Emu8086ModRM modrm;
    uint8_t      imm8;
    otype        imm;
    atype        count;
    atype        addr;

    switch (op)
    {
    case 0x00: /* ADD r/m8,r8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_modrm<uint8_t, uint8_t>(&modrm,
                                      op_add(read_modrm<uint8_t, uint8_t>(&modrm), read_reg<uint8_t>(modrm.reg)));
        break;
    case 0x01: /* ADD r/m16,r16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_modrm<otype, atype>(&modrm, op_add(read_modrm<otype, atype>(&modrm), read_reg<otype>(modrm.reg)));
        break;
    case 0x02: /* ADD r8,r/m8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_reg<uint8_t>(modrm.reg, op_add(read_reg<uint8_t>(modrm.reg), read_modrm<uint8_t, uint8_t>(&modrm)));
        break;
    case 0x03: /* ADD r16,r/m16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_reg<otype>(modrm.reg, op_add(read_reg<otype>(modrm.reg), read_modrm<otype, atype>(&modrm)));
        break;
    case 0x04: /* ADD AL,imm8 */
        read_ip<uint8_t>(&imm8);
        gEmu.regs[X86_EMU_EAX].u8 = op_add(gEmu.regs[X86_EMU_EAX].u8, imm8);
        break;
    case 0x05: /* ADD AX,imm16 */
        read_ip<otype>(&imm);
        write_reg<otype>(X86_EMU_EAX, op_add(read_reg<otype>(X86_EMU_EAX), imm));
        break;
    case 0x06: /* PUSH ES */
        push<otype>(gEmu.sregs[X86_EMU_ES]);
        break;
    case 0x07: /* POP ES */
        gEmu.sregs[X86_EMU_ES] = pop<otype>();
        break;
    case 0x08: /* OR r/m8,r8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_modrm<uint8_t, uint8_t>(&modrm,
                                      op_or(read_modrm<uint8_t, uint8_t>(&modrm), read_reg<uint8_t>(modrm.reg)));
        break;
    case 0x09: /* OR r/m16,r16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_modrm<otype, atype>(&modrm, op_or(read_modrm<otype, atype>(&modrm), read_reg<otype>(modrm.reg)));
        break;
    case 0x0a: /* OR r8,r/m8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_reg<uint8_t>(modrm.reg, op_or(read_reg<uint8_t>(modrm.reg), read_modrm<uint8_t, uint8_t>(&modrm)));
        break;
    case 0x0b: /* OR r16,r/m16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_reg<otype>(modrm.reg, op_or(read_reg<otype>(modrm.reg), read_modrm<otype, atype>(&modrm)));
        break;
    case 0x0c: /* OR AL,imm8 */
        read_ip<uint8_t>(&imm8);
        gEmu.regs[X86_EMU_EAX].u8 = op_or(gEmu.regs[X86_EMU_EAX].u8, imm8);
        break;
    case 0x0d: /* OR AX,imm16 */
        read_ip<otype>(&imm);
        write_reg<otype>(X86_EMU_EAX, op_or(read_reg<otype>(X86_EMU_EAX), imm));
        break;
    case 0x0e: /* PUSH CS */
        push<otype>(gEmu.sregs[X86_EMU_CS]);
        break;
    case 0x0f: /* POP CS */
        gEmu.sregs[X86_EMU_CS] = pop<otype>();
        break;
    case 0x10: /* ADC r/m8,r8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_modrm<uint8_t, uint8_t>(&modrm,
                                      op_adc(read_modrm<uint8_t, uint8_t>(&modrm), read_reg<uint8_t>(modrm.reg)));
        break;
    case 0x11: /* ADC r/m16,r16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_modrm<otype, atype>(&modrm, op_adc(read_modrm<otype, atype>(&modrm), read_reg<otype>(modrm.reg)));
        break;
    case 0x12: /* ADC r8,r/m8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_reg<uint8_t>(modrm.reg, op_adc(read_reg<uint8_t>(modrm.reg), read_modrm<uint8_t, uint8_t>(&modrm)));
        break;
    case 0x13: /* ADC r16,r/m16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_reg<otype>(modrm.reg, op_adc(read_reg<otype>(modrm.reg), read_modrm<otype, atype>(&modrm)));
        break;
    case 0x14: /* ADC AL,imm8 */
        read_ip<uint8_t>(&imm8);
        gEmu.regs[X86_EMU_EAX].u8 = op_adc(gEmu.regs[X86_EMU_EAX].u8, imm8);
        break;
    case 0x15: /* ADC AX,imm16 */
        read_ip<otype>(&imm);
        write_reg<otype>(X86_EMU_EAX, op_adc(read_reg<otype>(X86_EMU_EAX), imm));
        break;
    case 0x16: /* PUSH SS */
        push<otype>(gEmu.sregs[X86_EMU_SS]);
        break;
    case 0x17: /* POP SS */
        gEmu.sregs[X86_EMU_SS] = pop<otype>();
        break;
    case 0x18: /* SBB r/m8,r8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_modrm<uint8_t, uint8_t>(&modrm,
                                      op_add(read_modrm<uint8_t, uint8_t>(&modrm), read_reg<uint8_t>(modrm.reg)));
        break;
    case 0x19: /* SBB r/m16,r16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_modrm<otype, atype>(&modrm, op_add(read_modrm<otype, atype>(&modrm), read_reg<otype>(modrm.reg)));
        break;
    case 0x1a: /* SBB r8,r/m8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_reg<uint8_t>(modrm.reg, op_add(read_reg<uint8_t>(modrm.reg), read_modrm<uint8_t, uint8_t>(&modrm)));
        break;
    case 0x1b: /* SBB r16,r/m16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_reg<otype>(modrm.reg, op_add(read_reg<otype>(modrm.reg), read_modrm<otype, atype>(&modrm)));
        break;
    case 0x1c: /* SBB AL,imm8 */
        read_ip<uint8_t>(&imm8);
        gEmu.regs[X86_EMU_EAX].u8 = op_add(gEmu.regs[X86_EMU_EAX].u8, imm8);
        break;
    case 0x1d: /* SBB AX,imm16 */
        read_ip<otype>(&imm);
        write_reg<otype>(X86_EMU_EAX, op_add(read_reg<otype>(X86_EMU_EAX), imm));
        break;
    case 0x1e: /* PUSH DS */
        push<otype>(gEmu.sregs[X86_EMU_DS]);
        break;
    case 0x1f: /* POP DS */
        gEmu.sregs[X86_EMU_DS] = pop<otype>();
        break;
    case 0x20: /* AND r/m8,r8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_modrm<uint8_t, uint8_t>(&modrm,
                                      op_and(read_modrm<uint8_t, uint8_t>(&modrm), read_reg<uint8_t>(modrm.reg)));
        break;
    case 0x21: /* AND r/m16,r16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_modrm<otype, atype>(&modrm, op_and(read_modrm<otype, atype>(&modrm), read_reg<otype>(modrm.reg)));
        break;
    case 0x22: /* AND r8,r/m8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_reg<uint8_t>(modrm.reg, op_and(read_reg<uint8_t>(modrm.reg), read_modrm<uint8_t, uint8_t>(&modrm)));
        break;
    case 0x23: /* AND r16,r/m16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_reg<otype>(modrm.reg, op_and(read_reg<otype>(modrm.reg), read_modrm<otype, atype>(&modrm)));
        break;
    case 0x24: /* AND AL,imm8 */
        read_ip<uint8_t>(&imm8);
        gEmu.regs[X86_EMU_EAX].u8 = op_and(gEmu.regs[X86_EMU_EAX].u8, imm8);
        break;
    case 0x25: /* AND AX,imm16 */
        read_ip<otype>(&imm);
        write_reg<otype>(X86_EMU_EAX, op_and(read_reg<otype>(X86_EMU_EAX), imm));
        break;
    case 0x28: /* SUB r/m8,r8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_modrm<uint8_t, uint8_t>(&modrm,
                                      op_sub(read_modrm<uint8_t, uint8_t>(&modrm), read_reg<uint8_t>(modrm.reg)));
        break;
    case 0x29: /* SUB r/m16,r16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_modrm<otype, atype>(&modrm, op_sub(read_modrm<otype, atype>(&modrm), read_reg<otype>(modrm.reg)));
        break;
    case 0x2a: /* SUB r8,r/m8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_reg<uint8_t>(modrm.reg, op_sub(read_reg<uint8_t>(modrm.reg), read_modrm<uint8_t, uint8_t>(&modrm)));
        break;
    case 0x2b: /* SUB r16,r/m16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_reg<otype>(modrm.reg, op_sub(read_reg<otype>(modrm.reg), read_modrm<otype, atype>(&modrm)));
        break;
    case 0x2c: /* SUB AL,imm8 */
        read_ip<uint8_t>(&imm8);
        gEmu.regs[X86_EMU_EAX].u8 = op_sub(gEmu.regs[X86_EMU_EAX].u8, imm8);
        break;
    case 0x2d: /* SUB AX,imm16 */
        read_ip<otype>(&imm);
        write_reg<otype>(X86_EMU_EAX, op_sub(read_reg<otype>(X86_EMU_EAX), imm));
        break;
    case 0x30: /* XOR r/m8,r8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_modrm<uint8_t, uint8_t>(&modrm,
                                      op_xor(read_modrm<uint8_t, uint8_t>(&modrm), read_reg<uint8_t>(modrm.reg)));
        break;
    case 0x31: /* XOR r/m16,r16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_modrm<otype, atype>(&modrm, op_xor(read_modrm<otype, atype>(&modrm), read_reg<otype>(modrm.reg)));
        break;
    case 0x32: /* XOR r8,r/m8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_reg<uint8_t>(modrm.reg, op_xor(read_reg<uint8_t>(modrm.reg), read_modrm<uint8_t, uint8_t>(&modrm)));
        break;
    case 0x33: /* XOR r16,r/m16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_reg<otype>(modrm.reg, op_xor(read_reg<otype>(modrm.reg), read_modrm<otype, atype>(&modrm)));
        break;
    case 0x34: /* XOR AL,imm8 */
        read_ip<uint8_t>(&imm8);
        gEmu.regs[X86_EMU_EAX].u8 = op_xor(gEmu.regs[X86_EMU_EAX].u8, imm8);
        break;
    case 0x35: /* XOR AX,imm16 */
        read_ip<otype>(&imm);
        write_reg<otype>(X86_EMU_EAX, op_xor(read_reg<otype>(X86_EMU_EAX), imm));
        break;
    case 0x38: /* CMP r/m8,r8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        op_sub(read_modrm<uint8_t, uint8_t>(&modrm), read_reg<uint8_t>(modrm.reg));
        break;
    case 0x39: /* CMP r/m16,r16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        op_sub(read_modrm<otype, atype>(&modrm), read_reg<otype>(modrm.reg));
        break;
    case 0x3a: /* CMP r8,r/m8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        op_sub(read_reg<uint8_t>(modrm.reg), read_modrm<uint8_t, uint8_t>(&modrm));
        break;
    case 0x3b: /* CMP r16,r/m16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        op_sub(read_reg<otype>(modrm.reg), read_modrm<otype, atype>(&modrm));
        break;
    case 0x3c: /* CMP AL,imm8 */
        read_ip<uint8_t>(&imm8);
        op_sub(gEmu.regs[X86_EMU_EAX].u8, imm8);
        break;
    case 0x3d: /* CMP AX,imm16 */
        read_ip<otype>(&imm);
        op_sub(read_reg<otype>(X86_EMU_EAX), imm);
        break;
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47: /* INC r16 */
        write_reg<otype>(op & 7, op_inc(read_reg<otype>(op & 7), 1));
        break;
    case 0x48:
    case 0x49:
    case 0x4a:
    case 0x4b:
    case 0x4c:
    case 0x4d:
    case 0x4e:
    case 0x4f: /* DEC r16 */
        write_reg<otype>(op & 7, op_inc(read_reg<otype>(op & 7), -1));
        break;
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x53:
    case 0x54:
    case 0x55:
    case 0x56:
    case 0x57: /* PUSH r16 */
        push<otype>(gEmu.regs[op & 7].u32);
        break;
    case 0x58:
    case 0x59:
    case 0x5a:
    case 0x5b:
    case 0x5c:
    case 0x5d:
    case 0x5e:
    case 0x5f: /* POP r16 */
        write_reg<otype>(op & 7, pop<otype>());
        break;
    case 0x60: /* PUSHA */
        imm = gEmu.regs[X86_EMU_ESP].u32;
        push<otype>(gEmu.regs[X86_EMU_EAX].u32);
        push<otype>(gEmu.regs[X86_EMU_ECX].u32);
        push<otype>(gEmu.regs[X86_EMU_EDX].u32);
        push<otype>(gEmu.regs[X86_EMU_EBX].u32);
        push<otype>(imm);
        push<otype>(gEmu.regs[X86_EMU_EBP].u32);
        push<otype>(gEmu.regs[X86_EMU_ESI].u32);
        push<otype>(gEmu.regs[X86_EMU_EDI].u32);
        break;
    case 0x61: /* POPA */
        gEmu.regs[X86_EMU_EDI].u32 = pop<otype>();
        gEmu.regs[X86_EMU_ESI].u32 = pop<otype>();
        gEmu.regs[X86_EMU_EBP].u32 = pop<otype>();
        imm                        = pop<otype>();
        gEmu.regs[X86_EMU_EBX].u32 = pop<otype>();
        gEmu.regs[X86_EMU_EDX].u32 = pop<otype>();
        gEmu.regs[X86_EMU_ECX].u32 = pop<otype>();
        gEmu.regs[X86_EMU_EAX].u32 = pop<otype>();
        gEmu.regs[X86_EMU_ESP].u32 = imm;
        break;
    case 0x70: /* JO rel8 */
        op_jump<uint8_t>(test_flags(X86_FLAG_OF));
        break;
    case 0x71: /* JNO rel8 */
        op_jump<uint8_t>(!test_flags(X86_FLAG_OF));
        break;
    case 0x72: /* JB rel8 */
        op_jump<uint8_t>(test_flags(X86_FLAG_CF));
        break;
    case 0x73: /* JNB rel8 */
        op_jump<uint8_t>(!test_flags(X86_FLAG_CF));
        break;
    case 0x74: /* JZ rel8 */
        op_jump<uint8_t>(test_flags(X86_FLAG_ZF));
        break;
    case 0x75: /* JNZ rel8 */
        op_jump<uint8_t>(!test_flags(X86_FLAG_ZF));
        break;
    case 0x76: /* JNA rel8 */
        op_jump<uint8_t>(test_flags(X86_FLAG_CF | X86_FLAG_ZF));
        break;
    case 0x77: /* JA rel8 */
        op_jump<uint8_t>(!test_flags(X86_FLAG_CF | X86_FLAG_ZF));
        break;
    case 0x78: /* JS rel8 */
        op_jump<uint8_t>(test_flags(X86_FLAG_SF));
        break;
    case 0x79: /* JNS rel8 */
        op_jump<uint8_t>(!test_flags(X86_FLAG_SF));
        break;
    case 0x7a: /* JP rel8 */
        op_jump<uint8_t>(test_flags(X86_FLAG_PF));
        break;
    case 0x7b: /* JNP rel8 */
        op_jump<uint8_t>(!test_flags(X86_FLAG_PF));
        break;
    case 0x7c: /* JL rel8 */
        op_jump<uint8_t>(test_flags(X86_FLAG_SF) != test_flags(X86_FLAG_OF));
        break;
    case 0x7d: /* JNL rel8 */
        op_jump<uint8_t>(test_flags(X86_FLAG_SF) == test_flags(X86_FLAG_OF));
        break;
    case 0x7e: /* JLE rel8 */
        op_jump<uint8_t>(test_flags(X86_FLAG_ZF) || (test_flags(X86_FLAG_SF) != test_flags(X86_FLAG_OF)));
        break;
    case 0x7f: /* JNLE rel8 */
        op_jump<uint8_t>(!(test_flags(X86_FLAG_ZF) || (test_flags(X86_FLAG_SF) != test_flags(X86_FLAG_OF))));
        break;
    case 0x80:
    case 0x82:
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        read_ip<uint8_t>(&imm8);

        switch (modrm.reg)
        {
        case 0x0: /* ADD r/m8,imm8 */
            write_modrm<uint8_t, uint8_t>(&modrm, op_add(read_modrm<uint8_t, uint8_t>(&modrm), imm8));
            break;
        case 0x1: /* OR r/m8,imm8 */
            write_modrm<uint8_t, uint8_t>(&modrm, op_or(read_modrm<uint8_t, uint8_t>(&modrm), imm8));
            break;
        case 0x2: /* ADC r/m8,imm8 */
            write_modrm<uint8_t, uint8_t>(&modrm, op_adc(read_modrm<uint8_t, uint8_t>(&modrm), imm8));
            break;
        case 0x3: /* SBB r/m8,imm8 */
            write_modrm<uint8_t, uint8_t>(&modrm, op_sbb(read_modrm<uint8_t, uint8_t>(&modrm), imm8));
            break;
        case 0x4: /* AND r/m8,imm8 */
            write_modrm<uint8_t, uint8_t>(&modrm, op_and(read_modrm<uint8_t, uint8_t>(&modrm), imm8));
            break;
        case 0x5: /* SUB r/m8,imm8 */
            write_modrm<uint8_t, uint8_t>(&modrm, op_sub(read_modrm<uint8_t, uint8_t>(&modrm), imm8));
            break;
        case 0x6: /* XOR r/m8,imm8 */
            write_modrm<uint8_t, uint8_t>(&modrm, op_xor(read_modrm<uint8_t, uint8_t>(&modrm), imm8));
            break;
        case 0x7: /* CMP r/m8,imm8 */
            op_sub(read_modrm<uint8_t, uint8_t>(&modrm), imm8);
            break;
        }
        break;
    case 0x81:
        read_ipmodrm<atype>(&modrm, seg_override);
        read_ip<otype>(&imm);

        switch (modrm.reg)
        {
        case 0x0: /* ADD r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_add(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x1: /* OR r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_or(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x2: /* ADC r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_adc(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x3: /* SBB r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_sbb(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x4: /* AND r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_and(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x5: /* SUB r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_sub(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x6: /* XOR r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_xor(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x7: /* CMP r/m16,imm16 */
            op_sub(read_modrm<otype, atype>(&modrm), imm);
            break;
        }
        break;
    case 0x83:
        read_ipmodrm<atype>(&modrm, seg_override);
        read_ip<uint8_t>(&imm8);
        imm = sext32(imm8);

        switch (modrm.reg)
        {
        case 0x0: /* ADD r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_add(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x1: /* OR r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_or(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x2: /* ADC r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_adc(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x3: /* SBB r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_sbb(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x4: /* AND r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_and(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x5: /* SUB r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_sub(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x6: /* XOR r/m16,imm16 */
            write_modrm<otype, atype>(&modrm, op_xor(read_modrm<otype, atype>(&modrm), imm));
            break;
        case 0x7: /* CMP r/m16,imm16 */
            op_sub(read_modrm<otype, atype>(&modrm), imm);
            break;
        }
        break;
    case 0x84: /* TEST r/m8,r8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        op_and(read_modrm<uint8_t, uint8_t>(&modrm), read_reg<uint8_t>(modrm.reg));
        break;
    case 0x85: /* TEST r/m16,r16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        op_and(read_modrm<otype, atype>(&modrm), read_reg<otype>(modrm.reg));
        break;
    case 0x86: /* XCHG r8,r/m8 */
        read_ipmodrm<atype>(&modrm, seg_override);
        imm8 = read_reg<uint8_t>(modrm.reg);
        write_reg<uint8_t>(modrm.reg, read_modrm<uint8_t, uint8_t>(&modrm));
        write_modrm<uint8_t, uint8_t>(&modrm, imm8);
        break;
    case 0x87: /* XCHG r16,r/m16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        imm = read_reg<otype>(modrm.reg);
        write_reg<otype>(modrm.reg, read_modrm<otype, atype>(&modrm));
        write_modrm<otype, atype>(&modrm, imm);
        break;
    case 0x88: /* MOV r/m8,r8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_modrm<uint8_t, uint8_t>(&modrm, read_reg<uint8_t>(modrm.reg));
        break;
    case 0x89: /* MOV r/m16,r16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_modrm<otype, atype>(&modrm, read_reg<otype>(modrm.reg));
        break;
    case 0x8a: /* MOV r8,r/m8 */
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        write_reg<uint8_t>(modrm.reg, read_modrm<uint8_t, uint8_t>(&modrm));
        break;
    case 0x8b: /* MOV r16,r/m16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_reg<otype>(modrm.reg, read_modrm<otype, atype>(&modrm));
        break;
    case 0x8c: /* MOV Sreg,r/m16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_modrm<otype, atype>(&modrm, read_sreg(modrm.reg));
        break;
    case 0x8d: /* LEA r16,m */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_reg<otype>(modrm.reg, modrm_addr<atype>(&modrm));
        break;
    case 0x8e: /* MOV Sreg, r/m16 */
        read_ipmodrm<atype>(&modrm, seg_override);
        write_sreg(modrm.reg, read_modrm<otype, atype>(&modrm));
        break;
    case 0x90: /* NOP */
        break;
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97: /* XCHG AX,r16 */
        imm = read_reg<otype>(X86_EMU_EAX);
        write_reg<otype>(X86_EMU_EAX, read_reg<otype>(op & 7));
        write_reg<otype>(op & 7, imm);
        break;
    case 0x9c: /* PUSHF */
        push<otype>(gEmu.regs[X86_EMU_EFLAGS].u32 & 0xfcffff);
        break;
    case 0x9d: /* POPF */
        write_reg<otype>(X86_EMU_EFLAGS, pop<otype>());
        break;
    case 0xa2: /* MOV moffs8,AL */
        read_ip<atype>(&addr);
        write<uint8_t>(seg_addr(addr, gEmu.sregs[seg_override > -1 ? seg_override : X86_EMU_DS]),
                       read_reg<uint8_t>(X86_EMU_EAX));
        break;
    case 0xa3: /* MOV moffs16,AX */
        read_ip<atype>(&addr);
        write<otype>(seg_addr(addr, gEmu.sregs[seg_override > -1 ? seg_override : X86_EMU_DS]),
                     read_reg<otype>(X86_EMU_EAX));
        break;
    case 0xa4: /* MOVS m8 */
        addr_src = seg_addr_ind_autoinc<uint8_t, atype>(X86_EMU_DS, X86_EMU_ESI);
        addr_dst = seg_addr_ind_autoinc<uint8_t, atype>(X86_EMU_ES, X86_EMU_EDI);
        write<uint8_t>(addr_dst, read<uint8_t>(addr_src));
        break;
    case 0xa5: /* MOVS m16 */
        addr_src = seg_addr_ind_autoinc<otype, atype>(X86_EMU_DS, X86_EMU_ESI);
        addr_dst = seg_addr_ind_autoinc<otype, atype>(X86_EMU_ES, X86_EMU_EDI);
        write<otype>(addr_dst, read<otype>(addr_src));
        break;
    case 0xa8: /* TEST AL,imm8 */
        read_ip<uint8_t>(&imm8);
        op_and((uint8_t)gEmu.regs[X86_EMU_EAX].u32, imm8);
        break;
    case 0xa9: /* TEST AX,imm16 */
        read_ip<otype>(&imm);
        op_and((otype)gEmu.regs[X86_EMU_EAX].u32, imm);
        break;
    case 0xaa: /* STOS m8 */
        imm8     = read_reg<uint8_t>(X86_EMU_EAX);
        addr_dst = seg_addr_ind_autoinc<uint8_t, atype>(X86_EMU_ES, X86_EMU_EDI);
        write(addr_dst, imm8);
        break;
    case 0xab: /* STOS m16 */
        imm      = read_reg<otype>(X86_EMU_EAX);
        addr_dst = seg_addr_ind_autoinc<otype, atype>(X86_EMU_ES, X86_EMU_EDI);
        write(addr_dst, imm);
        break;
    case 0xac: /* LODS m8 */
        addr_src = seg_addr_ind_autoinc<uint8_t, atype>(X86_EMU_DS, X86_EMU_ESI);
        write_reg<uint8_t>(X86_EMU_EAX, read<uint8_t>(addr_src));
        break;
    case 0xad: /* LODS m16 */
        addr_src = seg_addr_ind_autoinc<otype, atype>(X86_EMU_DS, X86_EMU_ESI);
        write_reg<otype>(X86_EMU_EAX, read<otype>(addr_src));
        break;
    case 0xb0:
    case 0xb1:
    case 0xb2:
    case 0xb3:
    case 0xb4:
    case 0xb5:
    case 0xb6:
    case 0xb7: /* MOV r8,imm8 */
        read_ip<uint8_t>(&imm8);
        write_reg<uint8_t>(op & 0x7, imm8);
        break;
    case 0xb8:
    case 0xb9:
    case 0xba:
    case 0xbb:
    case 0xbc:
    case 0xbd:
    case 0xbe:
    case 0xbf: /* MOV r16, imm16 */
        read_ip<otype>(&imm);
        write_reg<otype>(op & 0x7, imm);
        break;
    case 0xc0:
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        read_ip<uint8_t>(&imm8);
        switch (modrm.reg)
        {
        case 4:
        case 6: /* SHL r/m8,imm8 */
            write_modrm<uint8_t, uint8_t>(&modrm, op_shl(read_modrm<uint8_t, uint8_t>(&modrm), imm8));
            break;
        case 5: /* SRL r/m8,imm8 */
            write_modrm<uint8_t, uint8_t>(&modrm, op_shr(read_modrm<uint8_t, uint8_t>(&modrm), imm8));
            break;
        default:
            NOT_IMPLEMENTED(modrm.reg);
            break;
        }
        break;
    case 0xc1:
        read_ipmodrm<atype>(&modrm, seg_override);
        read_ip<uint8_t>(&imm8);
        switch (modrm.reg)
        {
        case 4:
        case 6: /* SHL r/m16,imm8 */
            write_modrm<otype, atype>(&modrm, op_shl(read_modrm<otype, atype>(&modrm), (otype)imm8));
            break;
        case 5: /* SRL r/m16,imm8 */
            write_modrm<otype, atype>(&modrm, op_shr(read_modrm<otype, atype>(&modrm), (otype)imm8));
            break;
        default:
            NOT_IMPLEMENTED(modrm.reg);
            break;
        }
        break;
    case 0xc3: /* RET */
        gEmu.regs[X86_EMU_EIP].u32 = pop<otype>();
        break;
    case 0xcd: /* INT imm8 */
        read_ip<uint8_t>(&imm8);

        push<otype>(gEmu.regs[X86_EMU_EFLAGS].u32);
        push<otype>(gEmu.sregs[X86_EMU_CS]);
        push<otype>(gEmu.regs[X86_EMU_EIP].u32);
        gEmu.regs[X86_EMU_EFLAGS].u32 &= ~(X86_FLAG_IF | X86_FLAG_TF | X86_FLAG_AC);
        gEmu.regs[X86_EMU_EIP].u32 = ((uint16_t*)gEmu.ivt)[imm8 * 2 + 0];
        gEmu.sregs[X86_EMU_CS]     = ((uint16_t*)gEmu.ivt)[imm8 * 2 + 1];
        break;
    case 0xcf: /* IRET */
        gEmu.regs[X86_EMU_EIP].u32 = pop<otype>();
        gEmu.sregs[X86_EMU_CS]     = pop<otype>();
        write_reg<otype>(X86_EMU_EFLAGS, pop<otype>());

        if (seg_addr(gEmu.regs[X86_EMU_EIP].u16, gEmu.sregs[X86_EMU_CS]) == X86_EMU_NULLRET) return true;
        break;
    case 0xd1:
        read_ipmodrm<atype>(&modrm, seg_override);
        switch (modrm.reg)
        {
        case 4:
        case 6: /* SHL r/m16 */
            write_modrm<otype, atype>(&modrm, op_shl(read_modrm<otype, atype>(&modrm), (otype)1));
            break;
        case 5:
            write_modrm<otype, atype>(&modrm, op_shr(read_modrm<otype, atype>(&modrm), (otype)1));
            break;
        default:
            NOT_IMPLEMENTED(modrm.reg);
            break;
        }
        break;
    case 0xd3:
        read_ipmodrm<atype>(&modrm, seg_override);
        switch (modrm.reg)
        {
        case 4:
        case 6: /* SHL r/m16,CL */
            write_modrm<otype, atype>(&modrm,
                                      op_shl(read_modrm<otype, atype>(&modrm), (otype)gEmu.regs[X86_EMU_ECX].u8));
            break;
        case 5:
            write_modrm<otype, atype>(&modrm,
                                      op_shr(read_modrm<otype, atype>(&modrm), (otype)gEmu.regs[X86_EMU_ECX].u8));
            break;
        default:
            NOT_IMPLEMENTED(modrm.reg);
            break;
        }
        break;
    case 0xe4: /* IN AL,imm8 */
        read_ip<uint8_t>(&imm8);
        gEmu.regs[X86_EMU_EAX].u8 = op_in<uint8_t>(imm8);
        break;
    case 0xe5: /* IN AX,imm8 */
        read_ip<uint8_t>(&imm8);
        write_reg<otype>(X86_EMU_EAX, op_in<otype>(imm8));
        break;
    case 0xe6: /* OUT imm8,AL */
        read_ip<uint8_t>(&imm8);
        op_out<uint8_t>(imm8, gEmu.regs[X86_EMU_EAX].u8);
        break;
    case 0xe7: /* OUT imm8,AX */
        read_ip<uint8_t>(&imm8);
        op_out<otype>(imm8, read_reg<otype>(X86_EMU_EAX));
        break;
    case 0xe8: /* CALL rel16 */
        read_ip<otype>(&imm);
        push<otype>(gEmu.regs[X86_EMU_EIP].u32);
        gEmu.regs[X86_EMU_EIP].i32 += sext32(imm);
        break;
    case 0xe9: /* JMP rel16 */
        read_ip<otype>(&imm);
        gEmu.regs[X86_EMU_EIP].i32 += sext32(imm);
        break;
    case 0xeb: /* JMP rel8 */
        read_ip<uint8_t>(&imm8);
        gEmu.regs[X86_EMU_EIP].i32 += sext32(imm8);
        break;
    case 0xec: /* IN AL,DX */
        gEmu.regs[X86_EMU_EAX].u8 = op_in<uint8_t>(gEmu.regs[X86_EMU_EDX].u16);
        break;
    case 0xed: /* IN AX,DX */
        write_reg<otype>(X86_EMU_EAX, op_in<otype>(gEmu.regs[X86_EMU_EDX].u16));
        break;
    case 0xee: /* OUT DX,AL */
        op_out<uint8_t>(gEmu.regs[X86_EMU_EDX].u16, gEmu.regs[X86_EMU_EAX].u8);
        break;
    case 0xef: /* OUT DX,AX */
        op_out<otype>(gEmu.regs[X86_EMU_EDX].u16, read_reg<otype>(X86_EMU_EAX));
        break;
    case 0xf6:
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        switch (modrm.reg)
        {
        case 0:
        case 1: /* TEST r/m8,imm8 */
            read_ip<uint8_t>(&imm8);
            op_and(read_modrm<uint8_t, uint8_t>(&modrm), imm8);
            break;
        case 4: /* MUL AL,r/m8 */
            write_reg_ad<uint8_t>(
                op_mul<uint8_t>(read_reg<uint8_t>(X86_EMU_EAX), read_modrm<uint8_t, uint8_t>(&modrm)));
            break;
        case 5: /* IMUL AL,r/m8 */
            write_reg_ad<uint8_t>(
                op_imul<uint8_t>(read_reg<uint8_t>(X86_EMU_EAX), read_modrm<uint8_t, uint8_t>(&modrm)));
            break;
        default:
            NOT_IMPLEMENTED(modrm.reg);
        }
        break;
    case 0xf7:
        read_ipmodrm<atype>(&modrm, seg_override);
        switch (modrm.reg)
        {
        case 0:
        case 1: /* TEST r/m16,imm16 */
            read_ip<otype>(&imm);
            op_and(read_modrm<otype, atype>(&modrm), imm);
            break;
        case 4: /* MUL AX,r/m16 */
            write_reg_ad<otype>(op_mul<otype>(read_reg<otype>(X86_EMU_EAX), read_modrm<otype, atype>(&modrm)));
            break;
        case 5: /* IMUL AX,r/m16 */
            write_reg_ad<otype>(op_imul<otype>(read_reg<otype>(X86_EMU_EAX), read_modrm<otype, atype>(&modrm)));
            break;
        default:
            NOT_IMPLEMENTED(modrm.reg);
        }
        break;
    case 0xf8: /* CLC */
        gEmu.regs[X86_EMU_EFLAGS].u32 &= ~X86_FLAG_CF;
        break;
    case 0xf9: /* STC */
        gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_CF;
        break;
    case 0xfa: /* CLI */
        gEmu.regs[X86_EMU_EFLAGS].u32 &= ~X86_FLAG_IF;
        break;
    case 0xfb: /* STI */
        gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_IF;
        break;
    case 0xfc: /* CLD */
        gEmu.regs[X86_EMU_EFLAGS].u32 &= ~X86_FLAG_DF;
        break;
    case 0xfd: /* STD */
        gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_DF;
        break;
    case 0xfe:
        read_ipmodrm<uint8_t>(&modrm, seg_override);
        switch (modrm.reg)
        {
        case 0:
        case 2:
        case 4:
        case 6: /* INC r/m16 */
            write_modrm<uint8_t, uint8_t>(&modrm, op_inc(read_modrm<uint8_t, uint8_t>(&modrm), 1));
            break;
        case 1:
        case 3:
        case 5:
        case 7: /* DEC r/m16 */
            write_modrm<uint8_t, uint8_t>(&modrm, op_inc(read_modrm<uint8_t, uint8_t>(&modrm), -1));
            break;
        }
        break;
    case 0xff:
        read_ipmodrm<atype>(&modrm, seg_override);
        switch (modrm.reg)
        {
        case 0: /* INC r/m16 */
            write_modrm<otype, atype>(&modrm, op_inc(read_modrm<otype, atype>(&modrm), 1));
            break;
        case 1: /* DEC r/m16 */
            write_modrm<otype, atype>(&modrm, op_inc(read_modrm<otype, atype>(&modrm), -1));
            break;
        case 4: /* JMP r/m16 */
            write_reg<otype>(X86_EMU_EIP, read_modrm<otype, atype>(&modrm));
            break;
        case 6: /* PUSH r/m16 */
            push<otype>(read_modrm<otype, atype>(&modrm));
            break;
        default:
            NOT_IMPLEMENTED(modrm.reg);
        }
        break;
    case 0x180: /* JO rel8 */
        op_jump<otype>(test_flags(X86_FLAG_OF));
        break;
    case 0x181: /* JNO rel8 */
        op_jump<otype>(!test_flags(X86_FLAG_OF));
        break;
    case 0x182: /* JB rel8 */
        op_jump<otype>(test_flags(X86_FLAG_CF));
        break;
    case 0x183: /* JNB rel8 */
        op_jump<otype>(!test_flags(X86_FLAG_CF));
        break;
    case 0x184: /* JZ rel8 */
        op_jump<otype>(test_flags(X86_FLAG_ZF));
        break;
    case 0x185: /* JNZ rel8 */
        op_jump<otype>(!test_flags(X86_FLAG_ZF));
        break;
    case 0x186: /* JNA rel8 */
        op_jump<otype>(test_flags(X86_FLAG_CF | X86_FLAG_ZF));
        break;
    case 0x187: /* JA rel8 */
        op_jump<otype>(!test_flags(X86_FLAG_CF | X86_FLAG_ZF));
        break;
    case 0x188: /* JS rel8 */
        op_jump<otype>(test_flags(X86_FLAG_SF));
        break;
    case 0x189: /* JNS rel8 */
        op_jump<otype>(!test_flags(X86_FLAG_SF));
        break;
    case 0x18a: /* JP rel8 */
        op_jump<otype>(test_flags(X86_FLAG_PF));
        break;
    case 0x18b: /* JNP rel8 */
        op_jump<otype>(!test_flags(X86_FLAG_PF));
        break;
    case 0x18c: /* JL rel8 */
        op_jump<otype>(test_flags(X86_FLAG_SF) != test_flags(X86_FLAG_OF));
        break;
    case 0x18d: /* JNL rel8 */
        op_jump<otype>(test_flags(X86_FLAG_SF) == test_flags(X86_FLAG_OF));
        break;
    case 0x18e: /* JLE rel8 */
        op_jump<otype>(test_flags(X86_FLAG_ZF) || (test_flags(X86_FLAG_SF) != test_flags(X86_FLAG_OF)));
        break;
    case 0x18f: /* JNLE rel8 */
        op_jump<otype>(!(test_flags(X86_FLAG_ZF) || (test_flags(X86_FLAG_SF) != test_flags(X86_FLAG_OF))));
        break;
    case 0x1a0: /* PUSH FS */
        push<otype>(gEmu.sregs[X86_EMU_FS]);
        break;
    case 0x1a8: /* PUSH GS */
        push<otype>(gEmu.sregs[X86_EMU_GS]);
        break;
    case 0x2a4:
    case 0x4a4: /* REP MOVS m8 */
        count = read_reg<atype>(X86_EMU_ECX);
        for (atype i = 0; i < count; ++i)
        {
            addr_src = seg_addr_ind_autoinc<uint8_t, atype>(X86_EMU_DS, X86_EMU_ESI);
            addr_dst = seg_addr_ind_autoinc<uint8_t, atype>(X86_EMU_ES, X86_EMU_EDI);
            write<uint8_t>(addr_dst, read<uint8_t>(addr_src));
        }
        write_reg<atype>(X86_EMU_ECX, 0);
        break;
    case 0x2a5:
    case 0x4a5: /* REP MOVS m16 */
        count = read_reg<atype>(X86_EMU_ECX);
        for (atype i = 0; i < count; ++i)
        {
            addr_src = seg_addr_ind_autoinc<otype, atype>(X86_EMU_DS, X86_EMU_ESI);
            addr_dst = seg_addr_ind_autoinc<otype, atype>(X86_EMU_ES, X86_EMU_EDI);
            write<otype>(addr_dst, read<otype>(addr_src));
        }
        write_reg<atype>(X86_EMU_ECX, 0);
        break;
    case 0x2aa:
    case 0x4aa: /* REP STOS m8 */
        count = read_reg<atype>(X86_EMU_ECX);
        imm8  = read_reg<uint8_t>(X86_EMU_EAX);
        for (atype i = 0; i < count; ++i)
        {
            addr_dst = seg_addr_ind_autoinc<uint8_t, atype>(X86_EMU_ES, X86_EMU_EDI);
            write(addr_dst, imm8);
        }
        write_reg<atype>(X86_EMU_ECX, 0);
        break;
    case 0x2ab:
    case 0x4ab: /* REP STOS m16 */
        count = read_reg<atype>(X86_EMU_ECX);
        imm   = read_reg<otype>(X86_EMU_EAX);
        for (atype i = 0; i < count; ++i)
        {
            addr_dst = seg_addr_ind_autoinc<otype, atype>(X86_EMU_ES, X86_EMU_EDI);
            write(addr_dst, imm);
        }
        write_reg<atype>(X86_EMU_ECX, 0);
        break;
    case 0x2ac:
    case 0x4ac: /* REP LODS m8 */
        count = read_reg<atype>(X86_EMU_ECX);
        for (atype i = 0; i < count; ++i)
        {
            addr_src = seg_addr_ind_autoinc<uint8_t, atype>(X86_EMU_DS, X86_EMU_ESI);
            write_reg<uint8_t>(X86_EMU_EAX, read<uint8_t>(addr_src));
        }
        write_reg<atype>(X86_EMU_ECX, 0);
        break;
    case 0x2ad:
    case 0x4ad: /* REP LODS m16 */
        count = read_reg<atype>(X86_EMU_ECX);
        for (atype i = 0; i < count; ++i)
        {
            addr_src = seg_addr_ind_autoinc<otype, atype>(X86_EMU_DS, X86_EMU_ESI);
            write_reg<otype>(X86_EMU_EAX, read<otype>(addr_src));
        }
        write_reg<atype>(X86_EMU_ECX, 0);
        break;
    default:
        NOT_IMPLEMENTED(op);
        break;
    }
    return false;
}

static void emu8086_run()
{
    uint8_t  tmp;
    uint16_t op;
    bool     o32{};
    bool     a32{};
    uint16_t prefix_0f{};
    uint16_t prefix_rep{};
    bool     returned{};
    int8_t   seg_override{-1};

    while (!returned)
    {
        read_ip<uint8_t>(&tmp);

        switch (tmp)
        {
        case 0x26:
            seg_override = X86_EMU_ES;
            break;
        case 0x2e:
            seg_override = X86_EMU_CS;
            break;
        case 0x36:
            seg_override = X86_EMU_SS;
            break;
        case 0x3e:
            seg_override = X86_EMU_DS;
            break;
        case 0x64:
            seg_override = X86_EMU_FS;
            break;
        case 0x65:
            seg_override = X86_EMU_GS;
            break;
        case 0x66:
            o32 = true;
            break;
        case 0x67:
            a32 = true;
            break;
        case 0x0f:
            prefix_0f = 0x100;
            break;
        case 0xf2:
            prefix_rep = 0x200;
            break;
        case 0xf3:
            prefix_rep = 0x400;
            break;
        default:
            op = ((uint16_t)tmp | prefix_0f | prefix_rep);
            switch (((uint8_t)a32 << 1) | (uint8_t)o32)
            {
            case 0b00:
                returned = exec_instruction<uint16_t, uint16_t>(op, seg_override);
                break;
            case 0b01:
                returned = exec_instruction<uint32_t, uint16_t>(op, seg_override);
                break;
            case 0b10:
                returned = exec_instruction<uint16_t, uint32_t>(op, seg_override);
                break;
            case 0b11:
                returned = exec_instruction<uint32_t, uint32_t>(op, seg_override);
                break;
            }
            o32          = false;
            a32          = false;
            prefix_0f    = 0;
            prefix_rep   = 0;
            seg_override = -1;
            break;
        }
    }
}

void emu8086_init()
{
    memcpy(gEmu.ivt, physical_to_virtual(0x00000000), 0x400);
    memcpy(gEmu.bda, (uint8_t*)physical_to_virtual(0x00000000) + 0x400, 0x100);
    memcpy(gEmu.ebda, physical_to_virtual(0x00080000), 128 * 1024);
    gEmu.bios = (uint8_t*)physical_to_virtual(0xa0000);

    puts("Initialized 8086 emulator");
}

int emu8086_bios_int(int intnum, Emu8086BiosArgs* args)
{
    gEmu.regs[X86_EMU_EAX].u32    = args->eax;
    gEmu.regs[X86_EMU_EBX].u32    = args->ebx;
    gEmu.regs[X86_EMU_ECX].u32    = args->ecx;
    gEmu.regs[X86_EMU_EDX].u32    = args->edx;
    gEmu.regs[X86_EMU_ESI].u32    = args->esi;
    gEmu.regs[X86_EMU_EDI].u32    = args->edi;
    gEmu.regs[X86_EMU_EBP].u32    = 0x9000;
    gEmu.regs[X86_EMU_ESP].u32    = 0x9000 - 3 * 2;
    gEmu.regs[X86_EMU_EIP].u32    = ((uint16_t*)gEmu.ivt)[intnum * 2 + 0];
    gEmu.regs[X86_EMU_EFLAGS].u32 = 0x00000002;

    gEmu.sregs[X86_EMU_CS] = ((uint16_t*)gEmu.ivt)[intnum * 2 + 1];
    gEmu.sregs[X86_EMU_DS] = args->ds;
    gEmu.sregs[X86_EMU_ES] = args->es;
    gEmu.sregs[X86_EMU_FS] = args->fs;
    gEmu.sregs[X86_EMU_GS] = args->gs;
    gEmu.sregs[X86_EMU_SS] = 0x0000;

    write<uint16_t>(0x8ffe, 0x0002);
    write<uint16_t>(0x8ffc, 0x0000);
    write<uint16_t>(0x8ffa, X86_EMU_NULLRET);

    emu8086_run();

    args->eax = gEmu.regs[X86_EMU_EAX].u32;
    args->ebx = gEmu.regs[X86_EMU_EBX].u32;
    args->ecx = gEmu.regs[X86_EMU_ECX].u32;
    args->edx = gEmu.regs[X86_EMU_EDX].u32;
    args->esi = gEmu.regs[X86_EMU_ESI].u32;
    args->edi = gEmu.regs[X86_EMU_EDI].u32;

    args->ds = gEmu.sregs[X86_EMU_DS];
    args->es = gEmu.sregs[X86_EMU_ES];
    args->fs = gEmu.sregs[X86_EMU_FS];
    args->gs = gEmu.sregs[X86_EMU_GS];

    return !!(gEmu.regs[X86_EMU_EFLAGS].u32 & X86_FLAG_CF);
}

void emu8086_write(uint16_t seg, uint16_t base, void* addr, size_t size)
{
    uint32_t emu_addr;

    emu_addr = seg_addr(base, seg);
    for (size_t i = 0; i < size; ++i)
    {
        write<uint8_t>(emu_addr + i, ((uint8_t*)addr)[i]);
    }
}

void emu8086_read(void* addr, uint16_t seg, uint16_t base, size_t size)
{
    uint32_t emu_addr;

    emu_addr = seg_addr(base, seg);
    for (size_t i = 0; i < size; ++i)
    {
        ((uint8_t*)addr)[i] = read<uint8_t>(emu_addr + i);
    }
}
