#include <kernel/kernel.h>

#define NOT_IMPLEMENTED(x) \
    do                     \
    {                      \
        print(__FILE__);   \
        putchar(':');      \
        putu(__LINE__);    \
        print(" (");       \
        puthex16(x);       \
        print(")\n");      \
        for (;;)           \
        {                  \
        }                  \
    } while (0)

Emu8086 gEmu;

static uint32_t seg_addr(uint32_t base, uint16_t seg)
{
    return ((base & 0xffff) + seg * 16) & 0xfffff;
}

static uint8_t read_byte(uint32_t addr)
{
    addr &= 0xfffff;

    if (addr < 0x400)
        return gEmu.ivt[addr];
    if (addr >= 0x400 && addr < 0x500)
        return gEmu.bda[addr - 0x400];
    if (addr >= 0x8000 && addr < 0x9000)
        return gEmu.ram[addr - 0x8000];
    if (addr >= 0x80000 && addr < 0xa0000)
        return gEmu.ebda[addr - 0x80000];
    if (addr >= 0xa0000)
        return gEmu.bios[addr - 0xa0000];
    print("READ ERROR? ");
    puthex32(addr);
    putchar('\n');
    return 0;
}

template <typename T>
static T read(uint32_t addr)
{
    uint8_t tmp[sizeof(T)];
    T       value;

    for (size_t i = 0; i < sizeof(T); ++i)
    {
        tmp[i] = read_byte(addr + i);
    }
    memcpy(&value, tmp, sizeof(T));

    print("R");
    putu(sizeof(T));
    putchar(' ');
    puthex64(addr);
    print(" (");
    puthex32(value);
    print(")\n");

    return value;
}

static constexpr const auto read8  = read<uint8_t>;
static constexpr const auto read16 = read<uint16_t>;
static constexpr const auto read32 = read<uint32_t>;

static void write_byte(uint32_t addr, uint8_t value)
{
    addr &= 0xfffff;

    if (addr < 0x400)
        gEmu.ivt[addr] = value;
    else if (addr >= 0x400 && addr < 0x500)
        gEmu.bda[addr - 0x400] = value;
    else if (addr >= 0x8000 && addr < 0x9000)
        gEmu.ram[addr - 0x8000] = value;
    else if (addr >= 0x80000 && addr < 0xa0000)
        gEmu.ebda[addr - 0x80000] = value;
    else if (addr >= 0xa0000)
        gEmu.bios[addr - 0xa0000] = value;
    else
    {
        print("WRITE ERROR? ");
        puthex32(addr);
        putchar('\n');
    }
}

template <typename T>
static void write(uint32_t addr, T value)
{
    uint8_t tmp[sizeof(T)];

    print("W");
    putu(sizeof(T));
    putchar(' ');
    puthex64(addr);
    putchar('\n');

    memcpy(tmp, &value, sizeof(T));
    for (size_t i = 0; i < sizeof(T); ++i)
    {
        write_byte(addr + i, tmp[i]);
    }
}

static constexpr const auto write8  = write<uint8_t>;
static constexpr const auto write16 = write<uint16_t>;
static constexpr const auto write32 = write<uint32_t>;

template <typename T>
static void push(T value)
{
    gEmu.regs[X86_EMU_ESP].u32 -= sizeof(T);
    write<T>(seg_addr(gEmu.regs[X86_EMU_ESP].u32, gEmu.sregs[X86_EMU_SS]), value);
}

static constexpr const auto push8  = push<uint8_t>;
static constexpr const auto push16 = push<uint16_t>;
static constexpr const auto push32 = push<uint32_t>;

template <typename T>
static T pop(void)
{
    T tmp;

    tmp = read<T>(seg_addr(gEmu.regs[X86_EMU_ESP].u32, gEmu.sregs[X86_EMU_SS]));
    gEmu.regs[X86_EMU_ESP].u32 += sizeof(T);

    return tmp;
}

static constexpr const auto pop8  = pop<uint8_t>;
static constexpr const auto pop16 = pop<uint16_t>;
static constexpr const auto pop32 = pop<uint32_t>;

template <typename T>
static void read_ip(T* ptr)
{
    *ptr = read<T>(seg_addr(gEmu.regs[X86_EMU_EIP].u32, gEmu.sregs[X86_EMU_CS]));
    gEmu.regs[X86_EMU_EIP].u16 += sizeof(T);
}

static constexpr const auto read_ip8  = read_ip<uint8_t>;
static constexpr const auto read_ip16 = read_ip<uint16_t>;
static constexpr const auto read_ip32 = read_ip<uint32_t>;

static void read_ipmodrm16(Emu8086ModRM* ptr)
{
    read_ip8((uint8_t*)ptr);
    switch (ptr->mod)
    {
    case 0:
        /* Special case */
        if (ptr->rm == 6)
        {
            read_ip16((uint16_t*)&ptr->disp16);
        }
        break;
    case 1:
        read_ip8((uint8_t*)&ptr->disp8);
        break;
    case 2:
        read_ip16((uint16_t*)&ptr->disp16);
        break;
    case 3:
        break;
    }
}

static void read_ipmodrm32(Emu8086ModRM* ptr)
{
    read_ip8((uint8_t*)ptr);
    switch (ptr->mod)
    {
    case 0:
        break;
    case 1:
        read_ip8((uint8_t*)&ptr->disp8);
        break;
    case 2:
        read_ip32((uint32_t*)&ptr->disp32);
        break;
    case 3:
        break;
    }
}

template <typename atype>
static void read_ipmodrm(Emu8086ModRM* ptr)
{
    return read_ipmodrm16(ptr);
}

template <>
void read_ipmodrm<uint32_t>(Emu8086ModRM* ptr)
{
    return read_ipmodrm32(ptr);
}

template <typename otype>
static otype read_reg(uint8_t reg)
{
    return (otype)gEmu.regs[reg & 0x7].u32;
}

template <>
uint8_t read_reg<uint8_t>(uint8_t reg)
{
    uint16_t tmp;

    tmp = gEmu.regs[reg & 3].u16;
    tmp >>= ((reg & 0x4) ? 8 : 0);
    return tmp & 0xff;
}

template <typename T>
static void write_reg(uint8_t reg, T value)
{
    memcpy(&gEmu.regs[reg].u32, &value, sizeof(T));
}

template <>
void write_reg<uint8_t>(uint8_t reg, uint8_t value)
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

    if (reg > 5)
        return 0;
    return gEmu.sregs[reg];
}

static void write_sreg(uint8_t reg, uint16_t value)
{
    reg &= 0x7;

    if (reg > 5)
        return;
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
    }
    else
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

    if (gEmu.seg_override != -1)
    {
        seg = gEmu.sregs[gEmu.seg_override];
    }

    return seg_addr(base, seg);
}

static uint32_t modrm_addr16(const Emu8086ModRM* modrm)
{
    return modrm_addr8(modrm);
}

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

template <typename atype>
static uint32_t modrm_addr(const Emu8086ModRM* modrm)
{
    return modrm_addr16(modrm);
}

template <>
uint32_t modrm_addr<uint32_t>(const Emu8086ModRM* modrm)
{
    return modrm_addr32(modrm);
}

template <typename otype, typename atype>
static otype read_modrm(const Emu8086ModRM* modrm)
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

template <typename otype, typename atype>
static void write_modrm(const Emu8086ModRM* modrm, otype value)
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

template <typename T>
static T op_adc(T a, T b)
{
    static constexpr const uint32_t hi_bit = (1 << ((sizeof(T) * 8) - 1));

    int carryIn;
    T   res;
    T   carries;
    T   overflow;

    carryIn = (gEmu.regs[X86_EMU_EFLAGS].u32 & X86_FLAG_CF) ? 1 : 0;
    gEmu.regs[X86_EMU_EFLAGS].u32 &= ~(X86_FLAG_ZF | X86_FLAG_CF | X86_FLAG_OF | X86_FLAG_PF | X86_FLAG_AF | X86_FLAG_SF);

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

template <typename T>
static T op_sbb(T a, T b)
{
    T res;

    gEmu.regs[X86_EMU_EFLAGS].u32 ^= X86_FLAG_CF;
    res = op_adc<T>(a, ~b);
    gEmu.regs[X86_EMU_EFLAGS].u32 ^= X86_FLAG_CF;

    return res;
}

template <typename T>
static T op_add(T a, T b)
{
    gEmu.regs[X86_EMU_EFLAGS].u32 &= ~X86_FLAG_CF;
    return op_adc<T>(a, b);
}

template <typename T>
static T op_sub(T a, T b)
{
    T value;

    gEmu.regs[X86_EMU_EFLAGS].u32 &= ~X86_FLAG_CF;
    value = op_sbb<T>(a, b);

    print("SUB ");
    puthex32(a);
    print(" ");
    puthex32(b);
    print(" O:");
    putu(!!(gEmu.regs[X86_EMU_EFLAGS].u32 & X86_FLAG_OF));
    print(" C:");
    putu(!!(gEmu.regs[X86_EMU_EFLAGS].u32 & X86_FLAG_CF));
    putchar('\n');

    return value;
}

static constexpr const auto op_sub8  = op_sub<uint8_t>;
static constexpr const auto op_sub16 = op_sub<uint16_t>;
static constexpr const auto op_sub32 = op_sub<uint32_t>;

template <typename T>
static T op_logic(T res)
{
    static constexpr const uint32_t hi_bit = (1 << ((sizeof(T) * 8) - 1));

    gEmu.regs[X86_EMU_EFLAGS].u32 &= ~(X86_FLAG_ZF | X86_FLAG_CF | X86_FLAG_OF | X86_FLAG_PF | X86_FLAG_AF | X86_FLAG_SF);

    if (res == 0) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_ZF;
    if (res & hi_bit) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_SF;
    if (parity((uint8_t)res)) gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_PF;

    return res;
}

template <typename T>
static T op_xor(T a, T b)
{
    return op_logic<T>(a ^ b);
}

static constexpr const auto op_xor8  = op_xor<uint8_t>;
static constexpr const auto op_xor16 = op_xor<uint16_t>;
static constexpr const auto op_xor32 = op_xor<uint32_t>;

template <typename T>
static T op_and(T a, T b)
{
    return op_logic<T>(a & b);
}

static constexpr const auto op_and8  = op_and<uint8_t>;
static constexpr const auto op_and16 = op_and<uint16_t>;
static constexpr const auto op_and32 = op_and<uint32_t>;

template <typename T>
static T op_or(T a, T b)
{
    return op_logic<T>(a | b);
}

static constexpr const auto op_or8  = op_or<uint8_t>;
static constexpr const auto op_or16 = op_or<uint16_t>;
static constexpr const auto op_or32 = op_or<uint32_t>;

template <typename T>
static T op_shift(T original, T result, T carrymask, T ovmask)
{
    T   r;
    int carry;
    int overflow;

    r = op_logic<T>(result);

    carry    = !!(original & carrymask);
    overflow = !!(original & ovmask);

    if (carry)
        gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_CF;

    if (carry ^ overflow)
        gEmu.regs[X86_EMU_EFLAGS].u32 |= X86_FLAG_OF;

    return r;
}

template <typename T>
static T op_shl(T a, T b)
{
    return op_shift<T>(a, a << b, 1 << (sizeof(T) * 8 - b), 1 << (sizeof(T) * 8 - 2));
}

static constexpr const auto op_shl8  = op_shl<uint8_t>;
static constexpr const auto op_shl16 = op_shl<uint16_t>;
static constexpr const auto op_shl32 = op_shl<uint32_t>;

static bool test_flags(uint32_t flags)
{
    return ((gEmu.regs[X86_EMU_EFLAGS].u32 & flags) != 0);
}

static int32_t sext32(uint8_t v)
{
    return (int32_t)((int8_t)v);
}

static int32_t sext32(uint16_t v)
{
    return (int32_t)((int16_t)v);
}

static int32_t sext32(uint32_t v)
{
    return v;
}

template <typename T>
void op_jump(bool cond)
{
    T imm;

    read_ip<T>(&imm);
    if (cond) gEmu.regs[X86_EMU_EIP].i32 += sext32(imm);
}

template <typename otype, typename atype>
static bool exec_instruction(uint16_t op)
{
    Emu8086ModRM modrm;
    uint8_t      imm8;
    otype        imm;

    switch (op)
    {
    case 0x06: /* PUSH ES */
        push<otype>(gEmu.sregs[X86_EMU_ES]);
        break;
    case 0x07: /* POP ES */
        gEmu.sregs[X86_EMU_ES] = pop<otype>();
        break;
    case 0x08: /* OR r/m8, r8 */
        read_ipmodrm<uint8_t>(&modrm);
        write_modrm<uint8_t, uint8_t>(&modrm, op_or8(read_modrm<uint8_t, uint8_t>(&modrm), read_reg<uint8_t>(modrm.reg)));
        break;
    case 0x0e: /* PUSH CS */
        push<otype>(gEmu.sregs[X86_EMU_CS]);
        break;
    case 0xf: /* POP CS */
        gEmu.sregs[X86_EMU_CS] = pop<otype>();
        break;
    case 0x16: /* PUSH SS */
        push<otype>(gEmu.sregs[X86_EMU_SS]);
        break;
    case 0x17: /* POP SS */
        gEmu.sregs[X86_EMU_SS] = pop<otype>();
        break;
    case 0x1e: /* PUSH DS */
        push<otype>(gEmu.sregs[X86_EMU_DS]);
        break;
    case 0x1f: /* POP DS */
        gEmu.sregs[X86_EMU_DS] = pop<otype>();
        break;
    case 0x20: /* AND r/m8, r8 */
        read_ipmodrm<uint8_t>(&modrm);
        write_modrm<uint8_t, uint8_t>(&modrm, op_and8(read_modrm<uint8_t, uint8_t>(&modrm), read_reg<uint8_t>(modrm.reg)));
        break;
    case 0x2d: /* SUB ax,imm16 */
        read_ip<otype>(&imm);
        write_reg<otype>(X86_EMU_EAX, op_sub(read_reg<otype>(X86_EMU_EAX), imm));
        break;
    case 0x30: /* XOR r/m8,r8 */
        read_ipmodrm<uint8_t>(&modrm);
        write_modrm<uint8_t, uint8_t>(&modrm, op_xor8(read_modrm<uint8_t, uint8_t>(&modrm), read_reg<uint8_t>(modrm.reg)));
        break;
    case 0x3d: /* CMP AX, imm16 */
        read_ip<otype>(&imm);
        op_sub(read_reg<otype>(X86_EMU_EAX), imm);
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
        read_ipmodrm<uint8_t>(&modrm);
        read_ip8(&imm8);

        switch (modrm.reg)
        {
        case 0x4: /* AND r/m8, imm8 */
            write_modrm<uint8_t, uint8_t>(&modrm, op_and8(read_modrm<uint8_t, uint8_t>(&modrm), imm8));
            break;
        case 0x7: /* CMP r/m8, imm8 */
            op_sub8(read_modrm<uint8_t, uint8_t>(&modrm), imm8);
            break;
        default:
            NOT_IMPLEMENTED(modrm.reg);
            break;
        }
        break;
    case 0x88: /* MOV r/m8,r8 */
        read_ipmodrm<uint8_t>(&modrm);
        write_modrm<uint8_t, uint8_t>(&modrm, read_reg<uint8_t>(modrm.reg));
        break;
    case 0x89: /* MOV r/m16,r16 */
        read_ipmodrm<atype>(&modrm);
        write_modrm<otype, atype>(&modrm, read_reg<otype>(modrm.reg));
        break;
    case 0x8a: /* MOV r8,r/m8 */
        read_ipmodrm<uint8_t>(&modrm);
        write_reg<uint8_t>(modrm.reg, read_modrm<uint8_t, uint8_t>(&modrm));
        print("AL: ");
        puthex8(gEmu.regs[X86_EMU_EAX].u8);
        putchar('\n');
        break;
    case 0x8b: /* MOV r16,r/m16 */
        read_ipmodrm<atype>(&modrm);
        write_reg<otype>(modrm.reg, read_modrm<otype, atype>(&modrm));
        break;
    case 0x8e: /* MOV Sreg, r/m16 */
        read_ipmodrm<atype>(&modrm);
        write_sreg(modrm.reg, read_modrm<otype, atype>(&modrm));
        break;
    case 0x9c: /* PUSHF */
        push<otype>(gEmu.regs[X86_EMU_EFLAGS].u32 & 0xfcffff);
        break;
    case 0x9d: /* POPF */
        write_reg<otype>(X86_EMU_EFLAGS, pop<otype>());
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
    case 0xc3: /* RET */
        gEmu.regs[X86_EMU_EIP].u32 = pop<otype>();
        break;
    case 0xcf: /* IRET */
        gEmu.regs[X86_EMU_EIP].u32 = pop<otype>();
        gEmu.sregs[X86_EMU_CS]     = pop<otype>();
        write_reg<otype>(X86_EMU_EFLAGS, pop<otype>());

        print("iret ");
        puthex16(gEmu.sregs[X86_EMU_CS]);
        print(":");
        puthex16(gEmu.regs[X86_EMU_EIP].u16);
        putchar('\n');

        if (seg_addr(gEmu.regs[X86_EMU_EIP].u32, gEmu.sregs[X86_EMU_CS]) == X86_EMU_NULLRET)
            return true;
        break;
    case 0xd1:
        read_ipmodrm<atype>(&modrm);
        switch (modrm.reg)
        {
        case 4: /* SHL r/m16 */
            write_modrm<otype, atype>(&modrm, op_shl(read_modrm<otype, atype>(&modrm), (otype)1));
            break;
        default:
            NOT_IMPLEMENTED(modrm.reg);
            break;
        }
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
    bool     prefix_0f{};
    bool     returned{};

    while (!returned)
    {
        read_ip8(&tmp);

        switch (tmp)
        {
        case 0x66:
            o32 = true;
            break;
        case 0x67:
            a32 = true;
            break;
        case 0x0f:
            prefix_0f = true;
            break;
        default:
            op = ((uint16_t)tmp) | (prefix_0f ? 0x100 : 0x000);
            switch (((uint8_t)a32 << 1) | (uint8_t)o32)
            {
            case 0b00:
                returned = exec_instruction<uint16_t, uint16_t>(op);
                break;
            case 0b01:
                returned = exec_instruction<uint32_t, uint16_t>(op);
                break;
            case 0b10:
                returned = exec_instruction<uint16_t, uint32_t>(op);
                break;
            case 0b11:
                returned = exec_instruction<uint32_t, uint32_t>(op);
                break;
            }
            o32       = false;
            a32       = false;
            prefix_0f = false;
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

    Emu8086BiosArgs args;
    args.eax = 0x0001;
    emu8086_bios_int(0x10, &args);
}

void emu8086_bios_int(int intnum, Emu8086BiosArgs* args)
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
    gEmu.sregs[X86_EMU_DS] = 0x0000;
    gEmu.sregs[X86_EMU_ES] = 0x0000;
    gEmu.sregs[X86_EMU_FS] = 0x0000;
    gEmu.sregs[X86_EMU_GS] = 0x0000;
    gEmu.sregs[X86_EMU_SS] = 0x0000;

    write16(0x8ffe, 0x0002);
    write16(0x8ffc, 0x0000);
    write16(0x8ffa, X86_EMU_NULLRET);

    emu8086_run();
}
