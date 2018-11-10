#pragma once

#include <cstdint>
#include <array>
#include <istream>

using address = std::uint16_t;
using data    = std::uint8_t;

////////// Attributes used for MMIO

class MemoryInterfaceable {
public:
       MemoryInterfaceable(std::size_t size) : m_size(size) {};
       virtual ~MemoryInterfaceable() = default;

       std::size_t size() {return m_size;};

       virtual data  read(address offset)             = 0;
       virtual void write(address offset, data value) = 0;

protected:
       const std::size_t m_size = 0;
};

////////// ROM & RAM(s) : provides unified definition

template<std::size_t size>
class RAM : MemoryInterfaceable {
public:
       virtual data  read(address offset) override {return m_internal.at(offset);};
       virtual void write(address offset, data value) override {m_internal.at(offset) = value;};
protected:
       std::array<data, size> m_internal;
};

template<std::size_t size, typename lambda>
class BBRAM : RAM<size> {
public:
       BBRAM(lambda callback_destroyed) : callback(callback_destroyed) {};
       virtual ~BBRAM() {callback();};
protected:
       lambda callback;
};

template<std::size_t size>
class ROM : MemoryInterfaceable {
public:
       ROM(const std::istream* strm) {strm->read(m_internal.data(), size);
                                            if(strm->gcount() != size){throw std::runtime_error("Couldn't read \"size\" characters");}};

       virtual data  read(address offset) {return m_internal.at(offset);};
               void write(address offset, data value) {throw std::logic_error("Cannot write to ROM");};
private:
       std::array<data, size> m_internal;
};
