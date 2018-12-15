#pragma once

#include <array>

#include "gendefs.hpp"

template<int... data>
struct sequencer {
       int next() {int retval = m_data[m_index]; m_index = (m_index + 1) % m_data.size(); return retval;};
       void reset() {m_index = 0;}
protected:
       static constexpr std::array<std::uint8_t, sizeof...(data)> m_data {data...};
       int m_index = 0;
};
