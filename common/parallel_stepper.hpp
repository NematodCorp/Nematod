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
#include "tmputils.hpp"

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
    void reset() noexcept
    {
        void* clock_receivers[sizeof...(ClockReceivers)];
        for (size_t i = 0; i < sizeof...(ClockReceivers); ++i)
            clock_receivers[i] = m_coroutines[i].co.co->arg;
        for (auto& entry : m_coroutines)
        {
            destroy_co(entry.co);
        }
        unsigned co_idx = 0, receiver_idx = 0;
        ((m_coroutines[co_idx++] = {make_co(m_grp, &parallel_stepper_trampoline, clock_receivers[receiver_idx++]), ClockReceivers::clock_rate, 0}), ...);
    }

    void step() noexcept
    {
        static_for<sizeof...(ClockReceivers)>([this](auto n)
        {
            auto& entry = m_coroutines[n.value];
            ++entry.cur_clock;
            if (entry.cur_clock == entry.clock_rate)
            {
                entry.cur_clock = 0;
                run_co(entry.co);
            }
        });
    }

    void step_whole() noexcept
    {
        static_for<tvar_max(ClockReceivers::clock_rate...)>([this](auto clock)
        {
            int idx = 0;
            (run_if_clock<ClockReceivers, clock.value>(idx++), ...);
        });
    }

private:
    template <typename Receiver, size_t clock>
    inline constexpr void run_if_clock(int co_idx)
    {
        if constexpr ((clock % Receiver::clock_rate) == 0)
        {
            run_co(m_coroutines[co_idx].co);
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
