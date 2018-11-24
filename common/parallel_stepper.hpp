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

using stepper_func = void(*)();

template <stepper_func... Funcs>
class ParallelStepper
{
public:
    ParallelStepper() noexcept
        : m_grp(make_co_group()), m_coroutines{make_co(m_grp, Funcs)...}
    {

    }
    ~ParallelStepper() noexcept
    {
        for (auto& co : m_coroutines)
        {
            destroy_co(co);
        }
        destroy_co_group(m_grp);
    }

public:
    void set_arg(size_t idx, void* arg)
    {
        assert(idx < sizeof...(Funcs));
        set_co_arg(m_coroutines[idx], arg);
    }

    void step() noexcept
    {
        for (const auto& co : m_coroutines)
        {
            run_co(co);
        }
    }

public:
    coroutine_group m_grp;
    std::array<coroutine, sizeof...(Funcs)> m_coroutines;
};

#endif // PARALLEL_STEPPER_HPP
