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
    uint8_t addr = read(state.pc++);    cycle();
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::ZeroPageX>()
{
    uint8_t addr = read(state.pc++);    cycle();
    read(addr); addr += state.x;        cycle(); // dummy read
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::ZeroPageY>()
{    
    uint8_t addr = read(state.pc++);    cycle();
    read(addr); addr += state.y;        cycle(); // dummy read
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::Absolute>()
{
    uint16_t addr  = read(state.pc++);  cycle();
    addr |= read(state.pc++) << 8;      cycle();
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::AbsoluteX>()
{
    uint16_t addr  = read(state.pc++); cycle();
    addr |= read(state.pc++) << 8;     cycle();
    if ((addr&0xFF) + state.x >= 0x100) // page crossed
    {
        // do a dummy read at the invalid address
        read(addr&0xFF00 + (uint8_t)(addr&0xFF + state.x)); cycle();
    }
    addr += state.x;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::AbsoluteY>()
{
    uint16_t addr  = read(state.pc++); cycle();
    addr |= read(state.pc++) << 8;     cycle();
    if ((addr&0xFF) + state.y >= 0x100) // page crossed
    {
        // do a dummy read at the invalid address
        read(addr&0xFF00 + (uint8_t)(addr&0xFF + state.y)); cycle();
    }
    addr += state.y;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::AbsoluteXWrite>()
{
    uint16_t addr  = read(state.pc++); cycle();
    addr |= read(state.pc++) << 8;     cycle();
    // do a dummy read at the invalid address
    read(addr&0xFF00 + (uint8_t)(addr&0xFF + state.x)); cycle();
    addr += state.x;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::AbsoluteYWrite>()
{
    uint16_t addr  = read(state.pc++); cycle();
    addr |= read(state.pc++) << 8;     cycle();
    // do a dummy read at the invalid address
    read(addr&0xFF00 + (uint8_t)(addr&0xFF + state.y)); cycle();
    addr += state.y;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::IndZeroX>()
{
    uint8_t zero_addr  = read(state.pc++);      cycle();
    read(zero_addr); zero_addr += state.x;      cycle(); // dummy read
    uint16_t addr      = read(zero_addr);       cycle();
    addr     |= read(zero_addr+1)<<8;  cycle();

    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::IndZeroY>()
{
    uint8_t zero_addr  = read(state.pc++);      cycle();
    uint16_t addr      = read(zero_addr);       cycle();
    addr     |= read(zero_addr+1)<<8;  cycle();
    if ((addr&0xFF) + state.y >= 0x100) // page crossed
    {
        // do a dummy read at the invalid address
        read(addr&0xFF00 + (uint8_t)(addr&0xFF + state.y)); cycle();
    }
    addr += state.y;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::IndZeroYWrite>()
{
    uint8_t zero_addr  = read(state.pc++);      cycle();
    uint16_t addr      = read(zero_addr);       cycle();
    addr     |= read(zero_addr+1)<<8;  cycle();
    // do a dummy read at the invalid address
    read(addr&0xFF00 + (uint8_t)(addr&0xFF + state.y)); cycle();
    addr += state.y;
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::IndirectZP>()
{
    uint8_t zero_addr  = read(state.pc++); cycle();
    uint16_t addr      = read(zero_addr);  cycle();
    addr     |= read(zero_addr+1)<<8; cycle();

    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::Indirect>()
{
    uint16_t addr  = read(state.pc++);  cycle();
    addr |= read(state.pc++) << 8;      cycle();
    return addr;
}
template<>
uint16_t cpu6502::addr_mode_get<cpu6502::IndirectX>()
{
    uint16_t addr  = read(state.pc++);  cycle();
    addr |= read(state.pc++) << 8;      cycle();
    addr += state.x;                    cycle();
    return addr;
}

#endif // ADDR_MODES_HPP
