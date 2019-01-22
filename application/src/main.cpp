/*
nes.cpp

Copyright (c) 19 Yann BOUCHER (yann)

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

#include <cstring>

#include "core/include/nes.hpp"

#include "ppu/include/ppu.hpp"
#include "input/include/standard_controller.hpp"
#include "input/include/inputadapter.hpp"
#include "nesloader/include/nesloader.hpp"

#include <SFML/Graphics.hpp>

StandardController controller_1;

RAM<0x400 > ppu_nt1;
RAM<0x400 > ppu_nt2;
ROM<0x8000> prg_rom;
ROM<0x2000> chr_rom;
RAM<0x2000> chr_ram;
RAM<0x2000> crt_ram;

int main()
{
    coroutines_init();

    cartdrige_data cart;

    cart = load_nes_file("roms/smb.nes");

    NES::init();
    NES::input.controller_1 = &controller_1;

    memcpy(prg_rom.m_data.data(), cart.prg_rom.data(), cart.prg_rom.size());
    if (cart.prg_rom.size() == 0x4000)
    {
        memcpy(prg_rom.m_data.data() + 0x4000, cart.prg_rom.data(), cart.prg_rom.size());
    }

    if (!cart.chr_rom.empty())
        memcpy(chr_rom.m_data.data(), cart.chr_rom.data(), 0x2000);

    NES::cpu_space.add_port(memory_port{&crt_ram,  0x6000});
    NES::cpu_space.add_port(memory_port{&prg_rom , 0x8000});

    if (!cart.chr_rom.empty())
        NES::ppu.addr_space.add_port(memory_port{&chr_rom, 0x0000});
    else
        NES::ppu.addr_space.add_port(memory_port{&chr_ram, 0x0000});

    if (cart.mirroring == cartdrige_data::Horizontal)
    {
        NES::set_mirroring(NES::horizontal);
    }
    else if (cart.mirroring == cartdrige_data::Vertical)
    {
        // V-mirroring
        NES::set_mirroring(NES::vertical);
    }

    NES::power_cycle();

    crt_ram.m_internal[0x0] = 0x80;

#if 0
    while(crt_ram.m_internal[0x0] == 0x80)
    {
        NES::run_cpu_cycle();
    }
    printf("code : 0x%x, msg : '%s'\n", crt_ram.m_internal[0x0], &crt_ram.m_internal[0x4]);
    //printf("m_ctrl : 0x%x\n", ppu.m_ctrl);
    printf("pc : 0x%x, opcode is : 0x%x:'%s' (0x%x)\n", NES::cpu.state.pc, NES::cpu_space.read(NES::cpu.state.pc), cpu6502::opcode_mnemos[NES::cpu_space.read(NES::cpu.state.pc)], NES::cpu.state.pc);
    printf("ppu state : ctrl : 0x%x, mask : 0x%x, m_v : 0x%x, m_x : 0x%x\n", NES::ppu.m_ctrl, NES::ppu.m_mask, NES::ppu.m_v&0x7FF, NES::ppu.m_x);
    printf("cpu $06fc : 0x%x (0x%x)\n", NES::cpu_space.read(0x06fc), NES::cpu_space.read(0x074a));
    return;
#endif

    sf::RenderWindow window(sf::VideoMode(800, 600), "My window");

    sf::Image fb;
    fb.create(256, 240);

    sf::Texture texture;
    texture.loadFromImage(fb);

    sf::Sprite sprt;
    sprt.setTexture(texture);
    sprt.setScale(2,2);

    sf::Clock framerate_clock;

    std::array<uint32_t, 0x40> sfml_palette;

    for (size_t i { 0 }; i < 0x40; ++i)
    {
        sfml_palette[i] = __builtin_bswap32(PPU::ppu_palette[i] << 8 | 0xFF);
    }

    window.setFramerateLimit(60);

    // run the program as long as the window is open
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            bool press = true;
            switch (event.type)
            {
                // "close requested" event: we close the window
                case sf::Event::Closed:
                    window.close(); break;
                case sf::Event::KeyReleased:
                    press = false; [[fallthrough]];
                case sf::Event::KeyPressed:
                {
                    switch (event.key.code)
                    {
                        case sf::Keyboard::R:
                            NES::soft_reset();
                            break;
                        case sf::Keyboard::A:
                            controller_1.state.a = press; break;
                        case sf::Keyboard::B:
                            controller_1.state.b = press; break;
                        case sf::Keyboard::Q:
                            controller_1.state.select = press; break;
                        case sf::Keyboard::S:
                            controller_1.state.start = press; break;
                        case sf::Keyboard::Left:
                            if (press && controller_1.state.right) // prevent Left+Right presses
                                break;
                            controller_1.state.left = press; break;
                        case sf::Keyboard::Right:
                            if (press && controller_1.state.left)
                                break;
                            controller_1.state.right = press; break;
                        case sf::Keyboard::Up:
                            if (press && controller_1.state.down)
                                break;
                            controller_1.state.up = press; break;
                        case sf::Keyboard::Down:
                            if (press && controller_1.state.up)
                                break;
                            controller_1.state.down = press; break;
                        default:
                            break;
                    }
                }
                default: // Ignore other events
                    break;
            }
        }

        NES::run_frame();

        uint32_t* pixels_ptr = (uint32_t*)fb.getPixelsPtr();
        // take overscan in account : copy from line 8 to line 231
        for (size_t i { 0 }; i < 231*256; ++i)
        {
            pixels_ptr[i + 8*256] = sfml_palette[NES::ppu.framebuffer[i + 8*256]];
        }

        texture.update(fb);

        // clear the window with black color
        window.clear(sf::Color::Black);

        // draw everything here...
        window.draw(sprt);

        // end the current frame
        window.display();
    }
}
