/*
cpu65c02.hpp

Copyright (c) 26 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#ifndef CPU65C02_HPP
#define CPU65C02_HPP

#include <cstdint>
#include <array>

#include "common/bitops.hpp"
#include "common/coroutine.hpp"

struct OpcodeEntry;

enum cpu_type
{
    MOS6502,
    NES6502,
    WDC65c02
};

#ifndef CPU6502_FLAVOR
#define CPU6502_FLAVOR MOS6502
#endif
#ifndef CPU6502_CYCLE_ACTION
#define CPU6502_CYCLE_ACTION()
#endif

class cpu6502
{
public:
    static constexpr cpu_type flavor = CPU6502_FLAVOR;
    static const std::array<char[4], 256> opcode_mnemos;

public:
    using ReadCallback  = uint8_t(*)(uint16_t addr);
    using WriteCallback =    void(*)(uint16_t addr, uint8_t val);
    using LogCallback   =    void(*)(const char* str);

    cpu6502(ReadCallback in_read_clbk, WriteCallback in_write_clbk, LogCallback in_log_clbk = nullptr)
        : read_clbk(in_read_clbk), write_clbk(in_write_clbk), log_clbk(in_log_clbk)
    { }

public:
    void pull_irq_low();
    void raise_nmi();
    void reset();

    bool     stopped() const { return m_stopped; }

    void run(unsigned steps);

    struct state
    {
        uint8_t a;
        uint8_t x, y;
        uint8_t sp;
        uint8_t flags;
        uint16_t pc;
    }
    state;
    unsigned cycles { 0 };

    enum Flags
    {
        Carry = 1<<0,
        Zero  = 1<<1,
        IntD  = 1<<2,
        Decim = 1<<3,
        Break = 1<<4,
        Overf = 1<<6,
        Neg   = 1<<7
    };

public: /* private */
    uint8_t read(uint16_t addr)
    { return read_clbk(addr); }
    void    write(uint16_t addr, uint8_t val)
    { write_clbk(addr, val); }

    void    log(const char* str)
    { if (log_clbk) log_clbk(str); }

private:
    uint8_t fetch_opcode();

    bool carry() const
    { return state.flags & Carry; }
    bool zero() const
    { return state.flags & Zero; }
    bool interrupts_enabled() const
    { return !(state.flags & IntD); }
    bool decimal() const
    {
        if constexpr (flavor == NES6502) // no decimal mode on the NES's 6502
                return false;
        return state.flags & Decim;
    }
    bool is_break() const
    { return state.flags & Break; }
    bool overflow() const
    { return state.flags & Overf; }
    bool negative() const
    { return state.flags & Neg; }

    void branch_on(int8_t disp, bool cond);

    void set_carry(bool val) { bit_change(state.flags, val, 0); }
    void set_int_disable(bool val) { bit_change(state.flags, val, 2); }
    void set_decimal_mode(bool val) { bit_change(state.flags, val, 3); }
    void set_brk_flag(bool val) { bit_change(state.flags, val, 4); }
    void set_overflow(bool val) { bit_change(state.flags, val, 6); }
    void set_negative(bool val) { bit_change(state.flags, val, 7); }

    void test_zn(uint8_t val)
    {
#if 0
        state.flags &= ~0b10000010; // clear Z and S
        state.flags |= val & 0x80; // sign
        state.flags |= !!(val==0) << 1;
#else
        state.flags = (state.flags&~0b10000010) | ((val) ? (val & 0x80) : 0b10);
#endif
    }

    void push(uint8_t val);
    uint8_t pop();

    void switch_to_isr(uint16_t vector, bool brk = false);

public: /* private */
    enum AddrModes : unsigned
    {
        Implied,
        Immediate,
        ZeroPage,
        ZeroPageX,
        ZeroPageY,
        Absolute,
        AbsoluteX,
        AbsoluteY,
        IndZeroX,
        IndZeroY,
        IndirectZP,
        Indirect,
        IndirectX,
        BitRelative,
        BitZP,

        AddrModesCount,

        AbsoluteXWrite,
        AbsoluteYWrite,
        IndZeroYWrite,

        BusConflictInvalid
    };


    void cycle()
    {
        CPU6502_CYCLE_ACTION();
        ++cycles;
    }

    template<enum AddrModes>
    uint16_t addr_mode_get();

    void invalid_opcode();

    void adc(uint16_t);
    void sbc(uint16_t);
    void asl(uint16_t);
    void asla();
    void bbr(uint8_t bit, uint8_t val, int8_t branch);
    void bbs(uint8_t bit, uint8_t val, int8_t branch);
    void bcc(uint16_t);
    void bcs(uint16_t);
    void beq(uint16_t);
    void bit(uint16_t);
    void bmi(uint16_t);
    void bne(uint16_t);
    void bpl(uint16_t);
    void bra(uint16_t);
    void brk();
    void bvc(uint16_t);
    void bvs(uint16_t);
    void clc();
    void cld();
    void cli();
    void clv();
    void sec();
    void sed();
    void sei();
    void cmp(uint16_t);
    void cpx(uint16_t);
    void cpy(uint16_t);
    void inc(uint16_t);
    void inx();
    void iny();
    void inca();
    void dec(uint16_t);
    void dex();
    void dey();
    void deca();
    void eor(uint16_t);
    void ora(uint16_t);
    void and_(uint16_t);
    void jmp(uint16_t);
    void ind_jmp(uint16_t);
    void jsr(uint16_t);
    void lda(uint16_t);
    void ldx(uint16_t);
    void ldy(uint16_t);
    void sta(uint16_t);
    void stx(uint16_t);
    void sty(uint16_t);
    void stz(uint16_t);
    void lsra();
    void lsr(uint16_t);
    void rora();
    void ror(uint16_t);
    void rola();
    void rol(uint16_t);
    void nop();
    void nop2(uint16_t);
    void pha(); void pla();
    void phx(); void plx();
    void phy(); void ply();
    void php(); void plp();
    void rmb(uint8_t bit, uint8_t zp_addr);
    void smb(uint8_t bit, uint8_t zp_addr);
    void rti();
    void rts();
    void trb(uint16_t);
    void tsb(uint16_t);
    void tax();
    void txa();
    void tay();
    void tya();
    void tsx();
    void txs();
    void stp();
    void wai();

    // undocumented
    void alr(uint16_t);
    void anc(uint16_t);
    void arr(uint16_t);
    void axs(uint16_t);
    void lax(uint16_t);
    void sax(uint16_t);
    void dcp(uint16_t);
    void isc(uint16_t);
    void rla(uint16_t);
    void rra(uint16_t);
    void slo(uint16_t);
    void sre(uint16_t);
    void atx(uint16_t);

    void say(uint16_t);
    void xas(uint16_t);

private:
    ReadCallback read_clbk;
    WriteCallback write_clbk;
    LogCallback log_clbk;

    bool m_stopped { false };
    bool m_irq_pending { false };
    bool m_nmi_pending { false };
    bool m_int_delay   { false };
    bool m_wait_interrupt { false };
};

#endif // CPU65C02_HPP
