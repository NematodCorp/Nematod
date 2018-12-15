/*
parallel_stepper.hpp

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
#ifndef PARALLEL_STEPPER_HPP
#define PARALLEL_STEPPER_HPP

#include <array>
#include <cassert>

#include "coroutine.hpp"
#include "clock.hpp"

inline void parallel_stepper_trampoline()
{
    while (true)
    {
        auto rcv = (DividedClockReceiverInterface*)get_co_arg();
        rcv->on_active_clock();
    }
}

using stepper_func = void(*)();

template <typename... ClockReceivers>
class ParallelStepper
{
public:
    ParallelStepper(ClockReceivers&... args) noexcept
        : m_grp(make_co_group())
    {
        unsigned idx = 0;
        ((m_coroutines[idx++] = {make_co(m_grp, &parallel_stepper_trampoline, &args), ClockReceivers::clock_rate, 0}), ...);
    }
    ~ParallelStepper() noexcept
    {
        for (auto& entry : m_coroutines)
        {
            destroy_co(entry.co);
        }
        destroy_co_group(m_grp);
    }

public:
    void step() noexcept
    {
        for (auto& entry : m_coroutines)
        {
            ++entry.cur_clock;
            if (entry.cur_clock == entry.clock_rate)
            {
                entry.cur_clock = 0;
                run_co(entry.co);
            }
        }
    }

public:
    struct CoroutineEntry
    {
        coroutine co;
        unsigned clock_rate;
        unsigned cur_clock;
    };

    coroutine_group m_grp;
    std::array<CoroutineEntry, sizeof...(ClockReceivers)> m_coroutines;
};

#endif // PARALLEL_STEPPER_HPP
