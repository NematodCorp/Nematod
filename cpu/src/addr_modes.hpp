/*
addr_modes.hpp

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
#ifndef ADDR_MODES_HPP
#define ADDR_MODES_HPP

#include "cpu.hpp"

template<>
uint16_t cpu6502::addr_mode_get<cpu6502::Immediate>()
{
    return state.pc++;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::ZeroPage>()
{
    m_cycles += 1;

    uint8_t addr = read(state.pc++);
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::ZeroPageX>()
{
    m_cycles += 2;

    uint8_t addr = read(state.pc++);
    addr += state.x;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::ZeroPageY>()
{
    m_cycles += 2;

    uint8_t addr = read(state.pc++);
    addr += state.y;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::Absolute>()
{
    m_cycles += 2;

    uint16_t addr  = read(state.pc++);
    addr |= read(state.pc++) << 8;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::AbsoluteX>()
{
    m_cycles += 2;

    uint16_t addr  = read(state.pc++);
    addr |= read(state.pc++) << 8;
    if ((addr&0xFF) + state.x >= 0x100) // page crossed
    {
        m_cycles++;
    }
    addr += state.x;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::AbsoluteY>()
{
    m_cycles += 2;

    uint16_t addr  = read(state.pc++);
    addr |= read(state.pc++) << 8;
    if ((addr&0xFF) + state.y >= 0x100) // page crossed
    {
        m_cycles++;
    }
    addr += state.y;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::IndZeroX>()
{
    m_cycles += 4;


    uint8_t zero_addr  = read(state.pc++);
    zero_addr += state.x;
    uint16_t addr      = read(zero_addr);
             addr     |= read(zero_addr+1)<<8;

    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::IndZeroY>()
{
    m_cycles += 3;

    uint8_t zero_addr  = read(state.pc++);
    uint16_t addr      = read(zero_addr);
             addr     |= read(zero_addr+1)<<8;
    if ((addr&0xFF) + state.y >= 0x100) // page crossed
    {
        m_cycles++;
    }
    addr += state.y;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::IndirectZP>()
{
    m_cycles += 3;

    uint8_t zero_addr  = read(state.pc++);
    uint16_t addr      = read(zero_addr);
             addr     |= read(zero_addr+1)<<8;

    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::Indirect>()
{
    m_cycles += 3;

    uint16_t addr  = read(state.pc++);
    addr |= read(state.pc++) << 8;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::IndirectX>()
{
    m_cycles += 3;

    uint16_t addr  = read(state.pc++);
    addr |= read(state.pc++) << 8;
    addr += state.x;
    return addr;
}

#endif // ADDR_MODES_HPP
