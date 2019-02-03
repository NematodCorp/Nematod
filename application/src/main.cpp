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
#include "common/log.hpp"

#include <SFML/Graphics.hpp>

#include "tests/mappers/utils/screen_crc.hpp"

StandardController controller_1;

int main()
{
    coroutines_init();

    NES::init();
    bool result = NES::load_cartridge("roms/001/serom.nes");
    if (!result)
    {
        error("invalid rom\n");
        exit(1);
    }

    NES::input.controller_1 = &controller_1;

    if (NES::cart_data.battery_saved_ram)
    {
        NES::load_game_battery_save_data();
    }

    NES::power_cycle();

    sf::RenderWindow window(sf::VideoMode(800, 600), "My window");

    sf::Image fb;
    fb.create(256, 240);

    sf::Texture texture;
    texture.loadFromImage(fb);

    sf::Sprite sprt;
    sprt.setTexture(texture);
    sprt.setScale(2,2);

    sf::Clock framerate_clock;

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
            pixels_ptr[i + 8*256] = PPU::ppu_palette[NES::ppu.framebuffer[i + 8*256]];
        }

        texture.update(fb);

        // clear the window with black color
        window.clear(sf::Color::Black);

        // draw everything here...
        window.draw(sprt);

        // end the current frame
        window.display();
    }

    if (NES::cart_data.battery_saved_ram)
    {
        NES::save_game_battery_save_data();
    }

    printf("crc : 0x%X\n", screen_crc32());
}
