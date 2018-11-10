/*
opcode_table.cpp

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
#include <utility>
#include <variant>

#include "addr_modes.hpp"

struct OpcodeEntry
{
    const char name[4];
    std::variant<void(cpu6502::*)(uint16_t addr),
    void(cpu6502::*)(),
    void(cpu6502::*)(uint8_t imm),
    void(cpu6502::*)(uint8_t bit, uint8_t val, int8_t branch),
    void(cpu6502::*)(uint8_t bit, uint8_t zp_addr)> callback;
    uint8_t addrmode_ops[cpu6502::AddrModesCount];
};
using opcode_callback = void(*)(cpu6502&);

template<std::size_t N>
struct num { static const constexpr auto value = N; };

template <class F, std::size_t... Is>
constexpr void static_for_impl(F func, std::index_sequence<Is...>)
{
    (func(num<Is>{}), ...);
}

template <std::size_t N, typename F>
constexpr void static_for(F func)
{
    static_for_impl(func, std::make_index_sequence<N>());
}

constexpr OpcodeEntry opcode_defs[] = {
    /*   name      callback    impl imm/rel zp   zpx   zpy   abs   abx   aby   izx   izy   izp   ind   iax  bitrel bitzp */
    {"adc", &cpu6502::adc ,  {0x00, 0x69, 0x65, 0x75, 0x00, 0x6d, 0x7d, 0x79, 0x61, 0x71, 0x72, 0x00, 0x00, 0x00, 0x00} },
    {"and", &cpu6502::and_,  {0x00, 0x29, 0x25, 0x35, 0x00, 0x2d, 0x3d, 0x39, 0x21, 0x31, 0x32, 0x00, 0x00, 0x00, 0x00} },
    {"asl", &cpu6502::asl,   {0x00, 0x00, 0x06, 0x16, 0x00, 0x0e, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"asl", &cpu6502::asla,  {0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"bbr", &cpu6502::bbr ,  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00} },
    {"bbs", &cpu6502::bbs ,  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8F, 0x00} },
    {"bcc", &cpu6502::bcc ,  {0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"bcs", &cpu6502::bcs ,  {0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"beq", &cpu6502::beq ,  {0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"bit", &cpu6502::bit ,  {0x00, 0x89, 0x24, 0x34, 0x00, 0x2c, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"bmi", &cpu6502::bmi ,  {0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"bne", &cpu6502::bne ,  {0x00, 0xD0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"bpl", &cpu6502::bpl ,  {0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"bra", &cpu6502::bra ,  {0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"brk", &cpu6502::brk ,  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"bvc", &cpu6502::bvc ,  {0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"bvs", &cpu6502::bvs ,  {0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"clc", &cpu6502::clc ,  {0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"cld", &cpu6502::cld ,  {0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"cli", &cpu6502::cli ,  {0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"clv", &cpu6502::clv ,  {0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"cmp", &cpu6502::cmp ,  {0x00, 0xC9, 0xC5, 0xD5, 0x00, 0xCD, 0xDD, 0xD9, 0xC1, 0xD1, 0xD2, 0x00, 0x00, 0x00, 0x00} },
    {"cpx", &cpu6502::cpx ,  {0x00, 0xE0, 0xE4, 0x00, 0x00, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"cpy", &cpu6502::cpy ,  {0x00, 0xC0, 0xC4, 0x00, 0x00, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"dec", &cpu6502::dec ,  {0x00, 0x00, 0xC6, 0xD6, 0x00, 0xCE, 0xDE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"dec", &cpu6502::deca,  {0x3A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"dex", &cpu6502::dex ,  {0xCA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"dey", &cpu6502::dey ,  {0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"eor", &cpu6502::eor ,  {0x00, 0x49, 0x45, 0x55, 0x00, 0x4d, 0x5d, 0x59, 0x41, 0x51, 0x52, 0x00, 0x00, 0x00, 0x00} },
    /*   name      callback    impl imm/rel zp   zpx   zpy   abs   abx   aby   izx   izy   izp   ind   iax  bitrel bitzp */
    {"inc", &cpu6502::inc ,  {0x00, 0x00, 0xE6, 0xF6, 0x00, 0xEE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"inc", &cpu6502::inca,  {0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"inx", &cpu6502::inx ,  {0xE8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"iny", &cpu6502::iny ,  {0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"jmp", &cpu6502::jmp ,  {0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"jmp",&cpu6502::ind_jmp,{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6C, 0x7C, 0x00, 0x00} },
    {"jsr", &cpu6502::jsr ,  {0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"lda", &cpu6502::lda ,  {0x00, 0xA9, 0xA5, 0xB5, 0x00, 0xAD, 0xBD, 0xB9, 0xA1, 0xB1, 0xB2, 0x00, 0x00, 0x00, 0x00} },
    {"ldx", &cpu6502::ldx ,  {0x00, 0xA2, 0xA6, 0x00, 0xB6, 0xAE, 0x00, 0xBE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"ldy", &cpu6502::ldy ,  {0x00, 0xA0, 0xA4, 0xB4, 0x00, 0xAC, 0xBC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"lsr", &cpu6502::lsr ,  {0x00, 0x00, 0x46, 0x56, 0x00, 0x4E, 0x5E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"lsr", &cpu6502::lsra,  {0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"nop", &cpu6502::nop ,  {0xEA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"ora", &cpu6502::ora ,  {0x00, 0x09, 0x05, 0x15, 0x00, 0x0d, 0x1d, 0x19, 0x01, 0x11, 0x12, 0x00, 0x00, 0x00, 0x00} },
    {"pha", &cpu6502::pha ,  {0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"php", &cpu6502::php ,  {0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"phx", &cpu6502::phx ,  {0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"phy", &cpu6502::phy ,  {0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"pla", &cpu6502::pla ,  {0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"plp", &cpu6502::plp ,  {0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"plx", &cpu6502::plx ,  {0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"ply", &cpu6502::ply ,  {0x7A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"rmb", &cpu6502::rmb ,  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07} },
    {"smb", &cpu6502::smb ,  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87} },
    {"rol", &cpu6502::rol ,  {0x00, 0x00, 0x26, 0x36, 0x00, 0x2E, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"rol", &cpu6502::rola,  {0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"ror", &cpu6502::ror ,  {0x00, 0x00, 0x66, 0x76, 0x00, 0x6E, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"ror", &cpu6502::rora,  {0x6A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"rti", &cpu6502::rti ,  {0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"rts", &cpu6502::rts ,  {0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    /*   name      callback    impl imm/rel zp   zpx   zpy   abs   abx   aby   izx   izy   izp   ind   iax  bitrel bitzp */
    {"sbc", &cpu6502::sbc ,  {0x00, 0xE9, 0xE5, 0xF5, 0x00, 0xED, 0xFD, 0xF9, 0xE1, 0xF1, 0xF2, 0x00, 0x00, 0x00, 0x00} },
    {"sec", &cpu6502::sec ,  {0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"sed", &cpu6502::sed ,  {0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"sei", &cpu6502::sei ,  {0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"sta", &cpu6502::sta ,  {0x00, 0x00, 0x85, 0x95, 0x00, 0x8D, 0x9D, 0x99, 0x81, 0x91, 0x92, 0x00, 0x00, 0x00, 0x00} },
    {"stx", &cpu6502::stx ,  {0x00, 0x00, 0x86, 0x00, 0x96, 0x8E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"sty", &cpu6502::sty ,  {0x00, 0x00, 0x84, 0x94, 0x00, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"stp", &cpu6502::stp ,  {0xDB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"stz", &cpu6502::stz ,  {0x00, 0x00, 0x64, 0x74, 0x00, 0x9C, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"tax", &cpu6502::tax ,  {0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"tay", &cpu6502::tay ,  {0xA8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"trb", &cpu6502::trb ,  {0x00, 0x00, 0x14, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"tsb", &cpu6502::tsb ,  {0x00, 0x00, 0x04, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"tsx", &cpu6502::tsx ,  {0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"txs", &cpu6502::txs ,  {0x9A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"txa", &cpu6502::txa ,  {0x8A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"tya", &cpu6502::tya ,  {0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
    {"wai", &cpu6502::wai ,  {0xCB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
};

template <size_t idx, size_t mode>
constexpr opcode_callback generate_callback()
{
    return [](cpu6502& cpu)
    {
        if constexpr (mode == cpu6502::Implied)
        {
            (cpu.*std::get<1>(opcode_defs[idx].callback))();
        }
        else
        {
            uint16_t addr = cpu.addr_mode_get<(cpu6502::AddrModes)mode>();
            (cpu.*std::get<0>(opcode_defs[idx].callback))(addr);
        }
    };
}

template <size_t idx, size_t bit>
constexpr opcode_callback generate_bit_rel_callback()
{
    return [](cpu6502& cpu)
    {
        uint8_t    val = cpu.read(cpu.state.pc++);
        int8_t  branch = cpu.read(cpu.state.pc++);
        (cpu.*std::get<3>(opcode_defs[idx].callback))(bit, val, branch);
    };
}
template <size_t idx, size_t bit>
constexpr opcode_callback generate_bit_zp_callback()
{
    return [](cpu6502& cpu)
    {
        uint8_t    zp_addr = cpu.read(cpu.state.pc++);
        (cpu.*std::get<4>(opcode_defs[idx].callback))(bit, zp_addr);
    };
}

void invalid_opcode(cpu6502& cpu)
{
    cpu.invalid_opcode();
}

template <size_t idx>
constexpr void generate_opcode_callbacks(std::array<opcode_callback, 256>& opcodes)
{
    static_for<cpu6502::AddrModesCount>([&opcodes](auto mode)
    {
        if (uint8_t op = opcode_defs[idx].addrmode_ops[mode.value])
        {
            if constexpr (mode.value == cpu6502::BitRelative)
            {
                opcodes[op + 0x00] = generate_bit_rel_callback<idx, 0>();
                opcodes[op + 0x10] = generate_bit_rel_callback<idx, 1>();
                opcodes[op + 0x20] = generate_bit_rel_callback<idx, 2>();
                opcodes[op + 0x30] = generate_bit_rel_callback<idx, 3>();
                opcodes[op + 0x40] = generate_bit_rel_callback<idx, 4>();
                opcodes[op + 0x50] = generate_bit_rel_callback<idx, 5>();
                opcodes[op + 0x60] = generate_bit_rel_callback<idx, 6>();
                opcodes[op + 0x70] = generate_bit_rel_callback<idx, 7>();
            }
            else if (mode.value == cpu6502::BitZP)
            {
                opcodes[op + 0x00] = generate_bit_zp_callback<idx, 0>();
                opcodes[op + 0x10] = generate_bit_zp_callback<idx, 1>();
                opcodes[op + 0x20] = generate_bit_zp_callback<idx, 2>();
                opcodes[op + 0x30] = generate_bit_zp_callback<idx, 3>();
                opcodes[op + 0x40] = generate_bit_zp_callback<idx, 4>();
                opcodes[op + 0x50] = generate_bit_zp_callback<idx, 5>();
                opcodes[op + 0x60] = generate_bit_zp_callback<idx, 6>();
                opcodes[op + 0x70] = generate_bit_zp_callback<idx, 7>();
            }
            else
            {
                opcodes[op] = generate_callback<idx, mode.value>();
            }
        }
    });
}

constexpr std::array<opcode_callback, 256> gen()
{
    std::array<opcode_callback, 256> opcodes {};
    for (auto& el : opcodes) el = invalid_opcode;
    static_for<sizeof(opcode_defs)/sizeof(OpcodeEntry)>([&opcodes](auto i)
    {
        generate_opcode_callbacks<i.value>(opcodes);
    });
    opcodes[0x00] = [](cpu6502& cpu){ cpu.brk(); };
    return opcodes;
}

std::array<opcode_callback, 256> opcodes = gen();
