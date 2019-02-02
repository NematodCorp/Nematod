/*
nesloader.hpp

Copyright (c) 11 Yann BOUCHER (yann)

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
#ifndef NESLOADER_HPP
#define NESLOADER_HPP

#include <cstdint>
#include <vector>
#include <stdexcept>

struct cartridge_data
{
    enum MirroringType
    {
        Horizontal,
        Vertical,
        FourScreen
    };

    std::string          title;
    unsigned             mapper;
    unsigned             submapper;
    MirroringType        mirroring;
    unsigned long        prg_rom_size;
    unsigned long        chr_rom_size;
    std::vector<uint8_t> prg_rom;
    std::vector<uint8_t> chr_rom;
    unsigned long        prg_ram_size;
    unsigned long        nvram_size;
    unsigned long        chr_ram_size;
    unsigned long        chr_nvram_size;
    bool battery_saved_ram;
};

class cartridge_loader_error : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

bool           is_nes_file  (const std::vector<uint8_t>& file_data);
cartridge_data load_nes_file(const std::vector<uint8_t>& file_data, const std::string &title);
cartridge_data load_nes_file(const std::string& path);

#endif // NESLOADER_HPP
