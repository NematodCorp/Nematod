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

void cpu65c02::adc(uint16_t addr)
{

}

void cpu65c02::sbc(uint16_t addr)
{

}

void cpu65c02::and_(uint16_t addr)
{

}

void cpu65c02::asl(uint16_t addr)
{

}

void cpu65c02::asla()
{

}

void cpu65c02::bbr(uint8_t bit, uint8_t val, int8_t branch)
{

}

void cpu65c02::bbs(uint8_t bit, uint8_t val, int8_t branch)
{

}

void cpu65c02::bcc(uint16_t addr)
{
    branch_on(addr, !carry());
}

void cpu65c02::bcs(uint16_t addr)
{
    branch_on(addr, carry());
}

void cpu65c02::beq(uint16_t addr)
{
    branch_on(addr, zero());
}

void cpu65c02::bit(uint16_t addr)
{

}

void cpu65c02::bmi(uint16_t addr)
{
    branch_on(addr, negative());
}

void cpu65c02::bne(uint16_t addr)
{
    branch_on(addr, !zero());
}
void cpu65c02::bpl(uint16_t addr)
{
    branch_on(addr, !negative());
}

void cpu65c02::bra(uint16_t addr)
{
    branch_on(addr, true);
}

void cpu65c02::brk()
{

    ++state.pc; // brk takes two bytes
}

void cpu65c02::bvc(uint16_t addr)
{
    branch_on(addr, !overflow());
}

void cpu65c02::bvs(uint16_t addr)
{
    branch_on(addr, overflow());
}

void cpu65c02::clc()
{

}

void cpu65c02::cld(){}
void cpu65c02::cli(){}
void cpu65c02::clv(){}
void cpu65c02::sec(){}
void cpu65c02::sed(){}
void cpu65c02::sei(){}
void cpu65c02::cmp(uint16_t addr){}
void cpu65c02::cpx(uint16_t addr){}
void cpu65c02::cpy(uint16_t addr){}
void cpu65c02::inc(uint16_t addr){}
void cpu65c02::inx(){}
void cpu65c02::iny(){}
void cpu65c02::inca(){}
void cpu65c02::dec(uint16_t addr){}
void cpu65c02::dex(){}
void cpu65c02::dey(){}
void cpu65c02::deca(){}
void cpu65c02::eor(uint16_t addr){}
void cpu65c02::ora(uint16_t addr){}
void cpu65c02::jmp(uint16_t addr){}
void cpu65c02::jsr(uint16_t addr){}
void cpu65c02::lda(uint16_t addr){}
void cpu65c02::ldx(uint16_t addr){}
void cpu65c02::ldy(uint16_t addr){}
void cpu65c02::sta(uint16_t addr){}
void cpu65c02::stx(uint16_t addr){}
void cpu65c02::sty(uint16_t addr){}
void cpu65c02::stz(uint16_t addr){}
void cpu65c02::lsra(){}
void cpu65c02::lsr(uint16_t addr){}
void cpu65c02::rora(){}
void cpu65c02::ror(uint16_t addr){}
void cpu65c02::rola(){}
void cpu65c02::rol(uint16_t addr){}

void cpu65c02::nop()
{
}

void cpu65c02::pha(){}
void cpu65c02::pla(){}
void cpu65c02::phx(){}
void cpu65c02::plx(){}
void cpu65c02::phy(){}
void cpu65c02::ply(){}
void cpu65c02::php(){}
void cpu65c02::plp(){}
void cpu65c02::rmb(uint8_t bit, uint8_t zp_addr){}
void cpu65c02::smb(uint8_t bit, uint8_t zp_addr){}
void cpu65c02::rti(){}
void cpu65c02::rts(){}
void cpu65c02::trb(uint16_t addr){}
void cpu65c02::tsb(uint16_t addr){}
void cpu65c02::tax(){}
void cpu65c02::txa(){}
void cpu65c02::tay(){}
void cpu65c02::tya(){}
void cpu65c02::tsx(){}
void cpu65c02::txs(){}
void cpu65c02::stp(){}
void cpu65c02::wai(){}

void cpu65c02::invalid_opcode()
{
    if (log)
    {
        char buffer[64];
        snprintf(buffer, 64, "Invalid opcode 0x%02x at 0x%02x\n", read(state.pc-1), state.pc-1);
        log(buffer);
    }
}
