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

struct OpcodeEntry;

class cpu65c02
{
public:
    using ReadCallback  = uint8_t(*)(uint16_t addr);
    using WriteCallback =    void(*)(uint16_t addr, uint8_t val);
    using LogCallback   =    void(*)(const char* str);

    cpu65c02(ReadCallback read_clbk, WriteCallback write_clbk, LogCallback log = nullptr);

public:
    void irq();
    void nmi();
    void reset();

    unsigned cycles() const { return m_cycles; }

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

private:

    bool carry() const
    { return state.flags & Carry; }
    bool zero() const
    { return state.flags & Zero; }
    bool interrupts_enabled() const
    { return !(state.flags & IntD); }
    bool decimal() const
    { return state.flags & Decim; }
    bool is_break() const
    { return state.flags & Break; }
    bool overflow() const
    { return state.flags & Overf; }
    bool negative() const
    { return state.flags & Neg; }

    void branch_on(uint16_t addr, bool cond);

public: /* private */
    template<enum AddrModes : unsigned>
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

public:
    ReadCallback read;
    WriteCallback write;
    LogCallback log;
    unsigned m_cycles { 0 };
};

#endif // CPU65C02_HPP
