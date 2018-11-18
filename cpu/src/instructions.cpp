/*
instructions.cpp

Copyright (c) 27 Yann BOUCHER (yann)

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

#include "cpu.hpp"

#include <cstdio>

#include "common/bitops.hpp"

void cpu6502::adc(uint16_t addr)
{
    uint8_t value = read(addr);
    cycle();

    uint8_t result8 = 0;
    if (!decimal())
    {
        uint16_t result16 = (uint16_t)state.a + value + carry();
        set_carry(result16 > 255);
        result8  = result16 & 0xFF;
    }
    else // BCD Mode
    {
        uint8_t  lo = (value & 0xF ) + (state.a & 0xF ) + carry();
        uint16_t hi = (value & 0xF0) + (state.a & 0xF0);
        bool bcd_carry = false;
        if (lo > 0x9)
        {
            lo -= 0x0A;
            hi += 0x10;
        }
        if (hi > 0x90)
        {
            hi -= 0xA0;
            bcd_carry = true;
        }
        set_carry(bcd_carry);
        result8 = hi | (lo&0xF);

        cycles++;
    }

    set_overflow(~(state.a ^ value) & (state.a ^ result8) & 0x80);
    test_zn(result8);

    state.a = result8;
}

void cpu6502::sbc(uint16_t addr)
{
    uint8_t value = read(addr);
    cycle();

    uint8_t result8 = 0;
    if (!decimal())
    {
        int16_t result16 = (int16_t)state.a - value - !carry();
        set_carry(result16 >= 0);
        result8  = result16 & 0xFF;
    }
    else // BCD Mode
    {
        value = 0x99 - value;

        uint8_t  lo = (state.a & 0xF ) + (value & 0xF ) + carry();
        uint16_t hi = (state.a & 0xF0) + (value & 0xF0);
        bool bcd_carry = false;
        if (lo > 0x9)
        {
            lo -= 0x0A;
            hi += 0x10;
        }
        if (hi > 0x90)
        {
            hi -= 0xA0;
            bcd_carry = true;
        }
        set_carry(bcd_carry);
        result8 = hi | (lo&0xF);

        cycles++;
    }

    set_overflow((state.a ^ value) & (state.a ^ result8) & 0x80);
    test_zn(result8);

    state.a = result8;
}

void cpu6502::and_(uint16_t addr)
{
    uint8_t val = read(addr); cycle();

    state.a &= val;

    test_zn(state.a);
}

void cpu6502::asl(uint16_t addr)
{
    uint8_t val = read(addr); cycle();
    write(addr, val); cycle(); // dummy write
    set_carry(val & 0x80); // bit 7

    val <<= 1;

    test_zn(val);

    write(addr, val); cycle();
}

void cpu6502::asla()
{
    set_carry(state.a & 0x80); // bit 7

    state.a <<= 1;

    test_zn(state.a);
}

void cpu6502::bbr(uint8_t bit, uint8_t val, int8_t branch)
{
    uint16_t target = state.pc + branch;
    branch_on(target, bit_get(read(val), bit) == 0);
}

void cpu6502::bbs(uint8_t bit, uint8_t val, int8_t branch)
{
    uint16_t target = state.pc + branch;
    branch_on(target, bit_get(read(val), bit) == 1);
}

void cpu6502::bcc(uint16_t addr)
{
    branch_on(read(addr), !carry());
}

void cpu6502::bcs(uint16_t addr)
{
    branch_on(read(addr), carry());
}

void cpu6502::beq(uint16_t addr)
{
    branch_on(read(addr), zero());
}

void cpu6502::bit(uint16_t addr)
{
    uint8_t memory = read(addr); cycle();
    uint8_t value = state.a & memory;
    test_zn(value);
    set_overflow(bit_get(memory, 6));
    set_negative(bit_get(memory, 7));
}

void cpu6502::bmi(uint16_t addr)
{
    branch_on(read(addr), negative());
}

void cpu6502::bne(uint16_t addr)
{
    branch_on(read(addr), !zero());
}
void cpu6502::bpl(uint16_t addr)
{
    branch_on(read(addr), !negative());
}

void cpu6502::bra(uint16_t addr)
{
    branch_on(read(addr), true);
}

void cpu6502::brk()
{
    ++state.pc; // brk takes two bytes
    switch_to_isr(0xFFFE, true);
}

void cpu6502::bvc(uint16_t addr)
{
    branch_on(read(addr), !overflow());
}

void cpu6502::bvs(uint16_t addr)
{
    branch_on(read(addr), overflow());
}

void cpu6502::clc()
{
    set_carry(false);
}

void cpu6502::cld()
{
    set_decimal_mode(false);
}

void cpu6502::cli()
{
    set_int_disable(false);
    m_int_delay = true;
}

void cpu6502::clv()
{
    set_overflow(false);
}

void cpu6502::sec()
{
    set_carry(true);
}

void cpu6502::sed()
{
    set_decimal_mode(true);
}

void cpu6502::sei()
{
    set_int_disable(true);
    m_int_delay = true;
}

void cpu6502::cmp(uint16_t addr)
{
    uint8_t memory = read(addr); cycle();
    uint8_t result = state.a - memory;

    set_carry(state.a >= memory);
    test_zn(result);
}
void cpu6502::cpx(uint16_t addr)
{
    uint8_t memory = read(addr); cycle();
    uint8_t result = state.x - memory;

    set_carry(state.x >= memory);
    test_zn(result);
}
void cpu6502::cpy(uint16_t addr)
{
    uint8_t memory = read(addr); cycle();
    uint8_t result = state.y - memory;

    set_carry(state.y >= memory);
    test_zn(result);
}

void cpu6502::inc(uint16_t addr)
{
    uint8_t val = read(addr); cycle();
    write(addr, val); cycle(); // dummy write
    ++val;

    test_zn(val);

    write(addr, val); cycle();
}
void cpu6502::inx()
{
    ++state.x;

    test_zn(state.x);
}
void cpu6502::iny()
{
    ++state.y;

    test_zn(state.y);
}
void cpu6502::inca()
{
    ++state.a;

    test_zn(state.a);
}

void cpu6502::dec(uint16_t addr)
{
    uint8_t val = read(addr); cycle();
    write(addr, val); cycle(); // dummy write
    --val;

    test_zn(val);

    write(addr, val); cycle();
}
void cpu6502::dex()
{
    --state.x;

    test_zn(state.x);
}
void cpu6502::dey()
{
    --state.y;

    test_zn(state.y);
}
void cpu6502::deca()
{
    --state.a;

    test_zn(state.a);
}

void cpu6502::eor(uint16_t addr)
{
    uint8_t val = read(addr); cycle();

    state.a ^= val;

    test_zn(state.a);
}
void cpu6502::ora(uint16_t addr)
{
    uint8_t val = read(addr); cycle();

    state.a |= val;

    test_zn(state.a);
}

void cpu6502::jmp(uint16_t addr)
{
    state.pc = addr;
}

void cpu6502::ind_jmp(uint16_t addr)
{
    if constexpr (flavor == WDC65c02)
    {
        // fixed on later variants
        state.pc = 0;
        state.pc  = read(addr);         cycle();
        state.pc |= read(addr+1) << 8;  cycle();
    }
    else
    {
        // handle 6502's indirect jump bug
        uint16_t page = addr & 0xFF00;
        uint8_t  offset = addr & 0xFF;

        state.pc = 0;
        state.pc  = read(page + offset); cycle();
        state.pc |= read(page + (offset+1)) << 8; cycle();
    }
}

void cpu6502::jsr(uint16_t addr)
{
    uint16_t ret = state.pc - 1;

    push(ret >> 8);
    push(ret&0xFF);

    state.pc = addr;
    cycle();
}
void cpu6502::lda(uint16_t addr)
{
    state.a = read(addr); cycle();
    test_zn(state.a);
}
void cpu6502::ldx(uint16_t addr)
{
    state.x = read(addr); cycle();
    test_zn(state.x);
}
void cpu6502::ldy(uint16_t addr)
{
    state.y = read(addr); cycle();
    test_zn(state.y);
}
void cpu6502::sta(uint16_t addr)
{
    write(addr, state.a); cycle();
}
void cpu6502::stx(uint16_t addr)
{
    write(addr, state.x); cycle();
}
void cpu6502::sty(uint16_t addr)
{
    write(addr, state.y); cycle();
}
void cpu6502::stz(uint16_t addr)
{
    write(addr, 0); cycle();
}
void cpu6502::lsra()
{
    set_carry(state.a & 1);

    state.a >>= 1;

    test_zn(state.a);
}
void cpu6502::lsr(uint16_t addr)
{
    uint8_t val = read(addr); cycle();
    write(addr, val); cycle(); // dummy write
    set_carry(val & 1);

    val >>= 1;

    test_zn(val);

    write(addr, val); cycle();
}
void cpu6502::rola()
{
    bool old_carry = carry();
    set_carry(bit_get(state.a, 7));

    state.a <<= 1;
    bit_change(state.a, old_carry, 0);

    test_zn(state.a);
}
void cpu6502::rol(uint16_t addr)
{
    uint8_t val = read(addr); cycle();
    write(addr, val); cycle(); // dummy write

    bool old_carry = carry();
    set_carry(bit_get(val, 7));

    val <<= 1;
    bit_change(val, old_carry, 0);

    test_zn(val);

    write(addr, val); cycle();
}
void cpu6502::rora()
{
    bool old_carry = carry();
    set_carry(bit_get(state.a, 0));

    state.a >>= 1;
    bit_change(state.a, old_carry, 7);

    test_zn(state.a);
}
void cpu6502::ror(uint16_t addr)
{
    uint8_t val = read(addr); cycle();
    write(addr, val); cycle(); // dummy write

    bool old_carry = carry();
    set_carry(bit_get(val, 0));

    val >>= 1;
    bit_change(val, old_carry, 7);

    test_zn(val);

    write(addr, val); cycle();
}

void cpu6502::nop()
{
}

void cpu6502::pha()
{
    push(state.a);
}
void cpu6502::pla()
{
    cycle();
    state.a = pop();
    test_zn(state.a);
}
void cpu6502::phx()
{
    push(state.x);
}
void cpu6502::plx()
{
    cycle();
    state.x = pop();
    test_zn(state.x);
}
void cpu6502::phy()
{
    push(state.y);
}
void cpu6502::ply()
{
    cycle();
    state.y = pop();
    test_zn(state.y);
}
void cpu6502::php()
{
    if constexpr (flavor == WDC65c02)
    {
        push((state.flags | 0b10000) & ~0b1000); // BRK flag set and D flag clear
    }
    else
    {
        push(state.flags | 0b10000); // BRK flag set
    }
}
void cpu6502::plp()
{
    cycle();
    state.flags = pop() | 0b100000; // set bit 5
    m_int_delay = true;
}
void cpu6502::rmb(uint8_t bit, uint8_t zp_addr)
{
    uint8_t val = read(zp_addr);
    bit_clear(val, bit);
    write(zp_addr, val);
}
void cpu6502::smb(uint8_t bit, uint8_t zp_addr)
{
    uint8_t val = read(zp_addr);
    bit_set(val, bit);
    write(zp_addr, val);
}
void cpu6502::rti()
{
    cycle(); // increment S
    state.flags = (pop() & ~0b10000) | 0b100000; // clear BRK flag and set bit 5

    uint8_t low  = pop();
    uint8_t high = pop();
    state.pc = ((high << 8) | low);
}
void cpu6502::rts()
{
    cycle(); // <=> increment S
    uint8_t low  = pop();
    uint8_t high = pop();
    state.pc = ((high << 8) | low) + 1; cycle();
}
void cpu6502::trb(uint16_t addr)
{
    uint8_t val = read(addr);
    test_zn(state.a & val);
    val &= ~state.a;
    write(addr, val);
}
void cpu6502::tsb(uint16_t addr)
{
    uint8_t val = read(addr);
    test_zn(state.a & val);
    val |= state.a;
    write(addr, val);
}
void cpu6502::tax()
{
    state.x = state.a;
    test_zn(state.x);
}
void cpu6502::txa()
{
    state.a = state.x;
    test_zn(state.a);
}
void cpu6502::tay()
{
    state.y = state.a;
    test_zn(state.y);
}
void cpu6502::tya()
{
    state.a = state.y;
    test_zn(state.a);
}
void cpu6502::tsx()
{
    state.x = state.sp;
    test_zn(state.x);
}
void cpu6502::txs()
{
    state.sp = state.x;
}
void cpu6502::stp()
{
    m_stopped = true;
}
void cpu6502::wai()
{
    /* The above is true of an IRQ when the I (interrupt disable) flag is clear (i.e. interrupts are enabled).
     *  WAI is also useful with IRQs when the I flag is set (i.e. interrupts are disabled).
     * In this case, when an IRQ occurs (after the WAI instruction), the 65C02 will continue with with the next instruction rather than jumping to the interrupt vector.
     *  This means an IRQ can be responded to within one cycle!
     *  The interrupt handler is effectively inline code, rather than a separate routine, and thus it does not end with an RTI,
     * resulting in fewer cycles needed to handle the interrupt. */
    m_wait_interrupt = true;
}

void cpu6502::invalid_opcode()
{
    char buffer[64];
    snprintf(buffer, 64, "Invalid opcode 0x%02x at 0x%04x\n", read(state.pc-1), state.pc-1);
    log(buffer);
}
