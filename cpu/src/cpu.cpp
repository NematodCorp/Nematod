/*
cpu65c02.cpp

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

#include <array>
#include <cstdio>

#include "interface/memory.hpp"

extern const std::array<void(*)(cpu6502&), 256> opcodes;

void cpu6502::pull_nmi_low()
{
    if (m_nmi_line_state != 0)
    {
        raise_nmi();
    }
    m_nmi_line_state = 0;
}

void cpu6502::pull_nmi_high()
{
    m_nmi_line_state = 1;
}

void cpu6502::pull_irq_low()
{
    if (interrupts_enabled())
    {
        m_irq_pending = true;
    }
    if constexpr (flavor == cpu_type::WDC65c02)
    {
        m_wait_interrupt = false;
    }
}

void cpu6502::raise_nmi()
{
    m_nmi_pending = true;

    if constexpr (flavor == cpu_type::WDC65c02)
    {
        m_wait_interrupt = false;
    }
}

void cpu6502::reset()
{
    state.a = state.x = state.y = 0;
    state.flags = 0b00110100;
    state.sp = 0xFD;

    state.pc =  read(0xFFFC);
    state.pc |= read(0xFFFD) << 8;

    m_stopped        = false;
    m_int_delay      = false;
    m_wait_interrupt = false;
}

void cpu6502::run(unsigned steps)
{
    for (size_t i { 0 }; i < steps; ++i)
    {
        if constexpr (flavor == cpu_type::WDC65c02) // other versions don't have STP and WAI
        {
            if (stopped() || m_wait_interrupt)
            {
                return;
            }
        }

        m_int_delay = false;

        uint8_t opcode = fetch_opcode();
        cycle(); // first cycle : read opcode, increment PC

        auto operation = opcodes[opcode];
        operation(*this);

        // check interrupts
        if (!m_int_delay)
        {
            if (m_nmi_pending)
            {
                m_nmi_pending = false;
                switch_to_isr(0xFFFA);
            }
            if (m_irq_pending)
            {
                m_irq_pending = false;
                switch_to_isr(0xFFFE);
            }
        }
    }
}

uint8_t cpu6502::fetch_opcode()
{
    return read(state.pc++);
}

void cpu6502::switch_to_isr(uint16_t vector, bool brk)
{
    push(state.pc >> 8);
    push(state.pc & 0xFF);
    if constexpr (flavor == WDC65c02)
    {
        if (brk)
            push((state.flags & ~0b01000) | 0b10000); // with D flag unset
        else
            push(state.flags & ~0b11000); // with B and D flag unset
    }
    else
    {
        if (brk)
            push(state.flags | 0b10000);
        else
            push(state.flags & ~0b10000); // with B flag unset
    }

    uint16_t new_pc = 0;
    new_pc  = read(vector  ); set_int_disable(true); cycle();
    new_pc |= read(vector+1) << 8;                   cycle();

    state.pc = new_pc;
}

void cpu6502::branch_on(int8_t disp, bool cond)
{
    cycle();
    if (cond)
    {
        uint16_t addr = state.pc + disp;

        if ((addr&0xFF00) != (state.pc&0xFF00)) // diff page
        {
            cycle();
        }
        cycle();

        state.pc = addr;
    }
}

void cpu6502::push(uint8_t val)
{
    write(0x100 + state.sp, val);
    --state.sp;

    cycle();
}

uint8_t cpu6502::pop()
{
    ++state.sp;
    uint8_t val = read(0x100 + state.sp);

    cycle();

    return val;
}
