#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>

using data = std::uint8_t;

class illegal_read : public std::exception {};
