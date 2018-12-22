/*
tmputils.hpp

Copyright (c) 15 Yann BOUCHER (yann)

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
#ifndef TMPUTILS_HPP
#define TMPUTILS_HPP

#include <utility>
#include <type_traits>

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

template<typename T>
constexpr T tvar_max(T&&t)
{
  return std::forward<T>(t);
}

template<typename T0, typename T1, typename... Ts>
constexpr typename std::common_type<
  T0, T1, Ts...
>::type tvar_max(T0&& val1, T1&& val2, Ts&&... vs)
{
  if (val2 > val1)
    return tvar_max(val2, std::forward<Ts>(vs)...);
  else
    return tvar_max(val1, std::forward<Ts>(vs)...);
}


#endif // TMPUTILS_HPP
