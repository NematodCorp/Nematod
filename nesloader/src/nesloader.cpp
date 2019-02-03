/*
nesloader.cpp

Copyright (c) 17 Yann BOUCHER (yann)

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

#include "nesloader.hpp"

#include <cstring>
#include <cassert>

#include <fstream>

#include "common/fsutils.hpp"

void report_error(const std::string& str)
{
    throw cartridge_loader_error(str.c_str());
}

bool is_nes_file(const std::vector<uint8_t> &file_data)
{
    if (file_data.size() < 16) return false;

    return memcmp(file_data.data(), "NES\x1A", 4) == 0;
}

void load_nes2_data(const std::vector<uint8_t> &file_data, cartridge_data& cart)
{
    // mapper bits D8-D11
    cart.mapper |= (file_data[8]&0xF) << 8;
    cart.submapper = file_data[8]>>4;

    uint8_t prg_rom_msb = file_data[9]&0xF;
    uint8_t chr_rom_msb = file_data[9]>>4 ;

    cart.prg_rom_size |= prg_rom_msb << 8;
    if (prg_rom_msb == 0xF) // exponent notation
    {
        unsigned multiplier    = cart.prg_rom_size&0b11;
        unsigned long exponent = (cart.prg_rom_size&0b11111100)>>2;
        cart.prg_rom_size = (1 << exponent) * (multiplier*2 + 1);
    }

    cart.chr_rom_size |= chr_rom_msb << 8;
    if (chr_rom_msb == 0xF) // exponent notation
    {
        unsigned multiplier    = cart.chr_rom_size&0b11;
        unsigned long exponent = (cart.chr_rom_size&0b11111100)>>2;
        cart.chr_rom_size = (1 << exponent) * (multiplier*2 + 1);
    }

    unsigned shift_prg_ram, shift_nvram, shift_chr_ram, shift_chr_nvram;
    shift_prg_ram   = file_data[10]&0xF;
    shift_nvram     = file_data[10]>>4;
    shift_chr_ram   = file_data[11]&0xF;
    shift_chr_nvram = file_data[11]>>4;
    if (shift_prg_ram)
        cart.prg_ram_size   = 64 << shift_prg_ram;
    if (shift_nvram)
        cart.nvram_size     = 64 << shift_nvram;
    if (shift_chr_ram)
        cart.chr_ram_size   = 64 << shift_chr_ram;
    if (shift_chr_nvram)
        cart.chr_nvram_size = 64 << shift_chr_nvram;
}

cartridge_data load_nes_file(const std::vector<uint8_t> &file_data, const std::string& title)
{
    assert(is_nes_file(file_data));

    cartridge_data cart;
    cart.title = title;
    // 0 means 256
    cart.prg_rom_size = (file_data[4]?:256) * 0x4000;
    cart.chr_rom_size = file_data[5] * 0x2000;
    cart.mapper = (file_data[6] >> 4);
    cart.submapper = 0;

    if (file_data[6] & (1<<3))
        cart.mirroring = cartridge_data::MirroringType::FourScreen;
    else
        cart.mirroring = (file_data[6] & 0b1) ? cartridge_data::Vertical
                                              : cartridge_data::Horizontal;
    cart.battery_saved_ram = !!(file_data[6] & (1<<1));


    if (file_data.size() < 16 + cart.prg_rom.size() + cart.chr_rom.size())
        report_error("invalid iNES file");

    if (file_data[6] & (1<<2))
        report_error("trainers are unsupported");

    // iNES 1.0 header
    if ((file_data[7]&0x0C) == 0x00)
        cart.mapper |= (file_data[7] & 0xF0); // upper nibble was unsupported on early versions

    // NES 2.0 file
    if ((file_data[7]&0x0C) == 0x08)
        load_nes2_data(file_data, cart);
    else // set iNES 1.0 assumed values
    {
        cart.prg_ram_size = 0x2000; // default
    }

    cart.prg_rom.resize(cart.prg_rom_size);
    cart.chr_rom.resize(cart.chr_rom_size);

    memcpy(cart.prg_rom.data(), file_data.data() + 16 , cart.prg_rom.size());
    if (!cart.chr_rom.empty())
        memcpy(cart.chr_rom.data(), file_data.data() + 16 + cart.prg_rom.size(), cart.chr_rom.size());

    return cart;
}

cartridge_data load_nes_file(const std::string &path)
{
    std::ifstream file(path);
    if (!file)
        report_error("cannot load file '" + path + "'");

    file.seekg(0, std::ios::end );
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    if (!file.read((char*)data.data(), size))
        report_error("cannot load file '" + path + "'");

    auto title = std::string{trim_extension(filename(path))};

    return load_nes_file(data, title);
}
