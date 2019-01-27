/*
pathutils.hpp

Copyright (c) 27 Yann BOUCHER (yann)

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
#ifndef PATHUTILS_HPP
#define PATHUTILS_HPP

#include <string_view>
#include <string>
#include <fstream>
#include <vector>

template <class ContainerT = std::vector<std::string_view>>
inline ContainerT tokenize(std::string_view str, std::string_view delimiters = " ", bool trimEmpty = false)
{
    ContainerT tokens;
    tokens.reserve(str.size());

    std::string_view::size_type pos, lastPos = 0, length = str.length();

    using value_type = typename ContainerT::value_type;

    while(lastPos < length)
    {
        pos = str.find_first_of(delimiters, lastPos);
        if(pos == std::string_view::npos)
        {
            pos = length;
        }

        if(pos != lastPos || !trimEmpty)
            tokens.push_back(value_type(str.substr(lastPos, pos-lastPos)));

        lastPos = pos + 1;
    }

    return tokens;
}

inline std::string_view trim_extension(std::string_view filename)
{
    return filename.substr(0, filename.find_last_of('.'));
}

inline std::string_view filename(std::string_view path)
{
    return tokenize(path, "/").back();
}

inline bool exists(const std::string& path)
{
    std::ifstream file(path.c_str());
    return file.good();
}

#endif // PATHUTILS_HPP
