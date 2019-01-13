/*
sprite_0_test.cpp

Copyright (c) 22 Yann BOUCHER (yann)

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


#include "gtest/gtest.h"

#include <cstdio>
#include <cstring>

#include "common/parallel_stepper.hpp"
#include "clock.hpp"
#include "cpu.hpp"
#include "io_regs.hpp"
#include "standard_controller.hpp"
#include "ppu.hpp"
#include "ppu_regs.hpp"
#include "memory.hpp"
#include "nesloader.hpp"

#include <SFML/Graphics.hpp>

namespace
{

struct coroutines_init_wrapper
{
    coroutines_init_wrapper()
    {
        coroutines_init();
    }
};
static coroutines_init_wrapper co_init_instance;

AddressSpace cpu_space;

static uint8_t cpu6502_read(uint16_t addr)
{
    return cpu_space.read(addr);
}

extern cpu6502 cpu;

static void cpu6502_write(uint16_t addr, uint8_t val)
{
    if (addr >= 0x8000)
    {
        printf("MMC1 write ?\n");
    }
    cpu_space.write(addr, val);
}

cpu6502 cpu(cpu6502_read, cpu6502_write);
PPU ppu;
PPUCtrlRegs ppu_regs { ppu };
StandardController controller;
IORegs io_regs{cpu, nullptr, ppu, controller};


struct CPUReceiver : public DividedClockReceiver<12>
{
    void on_active_clock() override
    {
        cpu.run(1000);
    };
};
struct PPUReceiver : public DividedClockReceiver<4>
{
    void on_active_clock() override
    {
        ppu.render_frame();
    };
};

CPUReceiver cpu_rcv;
PPUReceiver ppu_rcv;

RAM<0x800 > nes_ram;
RAM<0x400 > ppu_nt1;
RAM<0x400 > ppu_nt2;
RAM<0x100 > palette_ram;
ROM<0x8000> prg_rom;
ROM<0x2000> chr_rom;
RAM<0x2000> chr_ram;
RAM<0x2000> crt_ram;

// for CNROM
ROM<0x2000> chr1_rom;
ROM<0x2000> chr2_rom;
ROM<0x2000> chr3_rom;
ROM<0x2000> chr4_rom;

TEST(Ppu, Sprite0)
{
    ppu.cpu = &cpu;

    cartdrige_data cart;
    ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_0_tests/01-basics.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_0_tests/02-alignment.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_0_tests/03-corners.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_0_tests/04-flip.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_0_tests/05-left_clip.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_0_tests/06-right_edge.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_0_tests/07-screen_bottom.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_0_tests/08-double_height.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_0_tests/09-timing.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_0_tests/10-timing_order.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_0_tests/loveiskind.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_0_tests/donkey kong.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/HelloWorld.nes"));

    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_overflow_tests/01-basics.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_overflow_tests/02-details.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_overflow_tests/03-timing.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_overflow_tests/04-obscure.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/sprite_overflow_tests/05-emulator.nes"));

    ASSERT_NO_THROW(cart = load_nes_file("roms/excitebike.nes"));

    //ASSERT_NO_THROW(cart = load_nes_file("roms/vbi_tests/02-vbl_set_time.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/vbi_tests/03-vbl_clear_time.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/vbi_tests/04-nmi_control.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/vbi_tests/05-nmi_timing.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/vbi_tests/06-suppression.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/vbi_tests/07-nmi_on_timing.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/vbi_tests/08-nmi_off_timing.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/vbi_tests/09-even_odd_frames.nes"));
    //ASSERT_NO_THROW(cart = load_nes_file("roms/vbi_tests/10-even_odd_timing.nes"));

    memcpy(prg_rom.data.data(), cart.prg_rom.data(), cart.prg_rom.size());
    memcpy(prg_rom.data.data() + 0x4000, cart.prg_rom.data(), cart.prg_rom.size());

    if (!cart.chr_rom.empty())
        memcpy(chr_rom.data.data(), cart.chr_rom.data(), 0x2000);

    cpu_space.add_port(memory_port{&nes_ram , 0x0000}); // RAM mirroring
    cpu_space.add_port(memory_port{&nes_ram , 0x0800});
    cpu_space.add_port(memory_port{&nes_ram , 0x1000});
    cpu_space.add_port(memory_port{&nes_ram , 0x1800});
    cpu_space.add_port(memory_port{&ppu_regs, 0x2000});
    cpu_space.add_port(memory_port{&io_regs,  0x4000});
    cpu_space.add_port(memory_port{&crt_ram,  0x6000});
    cpu_space.add_port(memory_port{&prg_rom , 0x8000});

    if (!cart.chr_rom.empty())
        ppu.addr_space.add_port(memory_port{&chr_rom, 0x0000});
    else
        ppu.addr_space.add_port(memory_port{&chr_ram, 0x0000});

    // H-mirroring
    ppu.addr_space.add_port(memory_port{&ppu_nt1, 0x2000});
    ppu.addr_space.add_port(memory_port{&ppu_nt2, 0x2400});
    ppu.addr_space.add_port(memory_port{&ppu_nt1, 0x2800});
    ppu.addr_space.add_port(memory_port{&ppu_nt2, 0x2C00});

    ppu.addr_space.add_port(memory_port{&palette_ram, 0x3F00});

    ParallelStepper stepper{cpu_rcv, ppu_rcv};

    io_regs.m_cpu_co = stepper.m_coroutines[0].co.co;

    cpu.reset();
    ppu.power_up();

    crt_ram.m_internal[0x0] = 0x80;

#if 0
    size_t clocks = 0;
    while(crt_ram.m_internal[0x0] == 0x80)
    {
        stepper.step_whole();
        //        clocks += 12;
        //        if (clocks >= 12886364) // 600 msec
        //        {
        //            ppu_regs.clear_decay();
        //            clocks = 0;
        //        }
    }
    printf("code : 0x%x, msg : '%s'\n", crt_ram.m_internal[0x0], &crt_ram.m_internal[0x4]);
    //printf("m_ctrl : 0x%x\n", ppu.m_ctrl);
    printf("pc : 0x%x, opcode is : 0x%x:'%s' (0x%x)\n", cpu.state.pc, cpu_space.read(cpu.state.pc), cpu6502::opcode_mnemos[cpu_space.read(cpu.state.pc)], cpu.state.pc);
    printf("ppu state : ctrl : 0x%x, mask : 0x%x, m_v : 0x%x, m_x : 0x%x\n", ppu.m_ctrl, ppu.m_mask, ppu.m_v&0x7FF, ppu.m_x);
    printf("cpu $06fc : 0x%x (0x%x)\n", cpu_space.read(0x06fc), cpu_space.read(0x074a));
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
                            cpu.reset(); ppu_regs.reset(); ppu.reset();
                            stepper.reset(); // reset coroutines state
                            break;
                        case sf::Keyboard::A:
                            controller.p1_state.a = press; break;
                        case sf::Keyboard::B:
                            controller.p1_state.b = press; break;
                        case sf::Keyboard::Q:
                            controller.p1_state.select = press; break;
                        case sf::Keyboard::S:
                            controller.p1_state.start = press; break;
                        case sf::Keyboard::Left:
                            if (press && controller.p1_state.right) // prevent Left+Right presses
                                break;
                                controller.p1_state.left = press; break;
                        case sf::Keyboard::Right:
                            if (press && controller.p1_state.left)
                                break;
                                controller.p1_state.right = press; break;
                        case sf::Keyboard::Up:
                            if (press && controller.p1_state.down)
                                break;
                                controller.p1_state.up = press; break;
                        case sf::Keyboard::Down:
                            if (press && controller.p1_state.up)
                                break;
                                controller.p1_state.down = press; break;
                    }
                }
            }
        }
        // run until vblank then draw frame to prevent rendering artifacts
        for (size_t i { 0 }; i < (341*4/12)*241; ++i) // cpu clocks per scanline * 241
        {
            stepper.step_whole();
        }
        // run until scanline 241 is actually reached
        while (ppu.m_current_line != 241)
        {
            stepper.step_whole();
        }

        uint32_t* pixels_ptr = (uint32_t*)fb.getPixelsPtr();
        // take overscan in account : copy from line 8 to line 231
        for (size_t i { 0 }; i < 231*256; ++i)
        {
            pixels_ptr[i + 8*256] = sfml_palette[ppu.framebuffer[i + 8*256]];
        }

        texture.update(fb);

        // clear the window with black color
        window.clear(sf::Color::Black);

        // draw everything here...
        window.draw(sprt);

        // end the current frame
        window.display();
    }

    for (size_t i { 0 }; i < 240/8; ++i)
    {
        for (size_t j { 0 }; j < 256/8; ++j)
        {
            printf("%.2X ", ppu.addr_space.read(0x2000 + i*(256/8) + j));
        }
        printf("\n");
    }
    printf("CHR : \n");
    for (size_t i { 0 }; i < 0x100; ++i)
    {
        printf("%.2X ", ppu.addr_space.read(0x0000 + i));
        if (i % 16 == 15)
            printf("\n");
    }
    printf("Palette : \n");
    for (size_t i { 0 }; i < 0x40; ++i)
    {
        printf("%.2X ", ppu.addr_space.read(0x3F00 + i));
        if (i % 16 == 15)
            printf("\n");
    }

    printf("code : 0x%x, msg : '%s'\n", crt_ram.m_internal[0x0], &crt_ram.m_internal[0x4]);
    //printf("m_ctrl : 0x%x\n", ppu.m_ctrl);
    printf("pc : 0x%x, opcode is : 0x%x:'%s' (0x%x)\n", cpu.state.pc, cpu_space.read(cpu.state.pc), cpu6502::opcode_mnemos[cpu_space.read(cpu.state.pc)], cpu.state.pc);
    printf("ppu state : ctrl : 0x%x, mask : 0x%x, m_v : 0x%x, m_x : 0x%x\n", ppu.m_ctrl, ppu.m_mask, ppu.m_v&0x7FF, ppu.m_x);
    printf("cpu $06fc : 0x%x (0x%x)\n", cpu_space.read(0x06fc), cpu_space.read(0x074a));
}

}
