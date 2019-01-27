/*
mmc1.hpp

Copyright (c) 23 Yann BOUCHER (yann)

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
#ifndef MMC1_HPP
#define MMC1_HPP

#include "mapper_base.hpp"

#include <vector>

class MMC1 : public Mapper
{
public:
    virtual void init(const cartridge_data& cart) override;

    void register_write(uint16_t addr, uint8_t val);

    virtual void load_battery_ram(const std::vector<uint8_t>& data) override;
    virtual std::vector<uint8_t> save_battery_ram() override;

private:
    void apply_banking();

private:
    std::vector<uint8_t> prg_rom;
    std::vector<uint8_t> prg_ram;
    std::vector<uint8_t> chr_rom;

    uint8_t write_count { 0 };
    uint8_t shift_register { 0 };
    uint8_t ctrl_reg { 0 };
    uint8_t chr0_reg { 0 };
    uint8_t chr1_reg { 0 };
    uint8_t prg_reg  { 0 };
    uint8_t last_write_cycle { 0 };
};
extern MMC1 mmc1;

#endif // MMC1_HPP
