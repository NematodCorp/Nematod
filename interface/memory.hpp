#pragma once

#include <cstdint>
#include <array>
#include <istream>

using address = std::uint16_t;
using data    = std::uint8_t;

class Memory_interface {
public:
       virtual data  read(address addr) = 0;
       virtual void write(address addr, data value) = 0;
};

////////// Attributes used for MMIO

class MemoryInterfaceable {
public:
       MemoryInterfaceable(std::size_t size) : m_size(size) {};
       virtual ~MemoryInterfaceable() = default;

       std::size_t size() {return m_size;};

       virtual data  read(address offset)             = 0;
       virtual void write(address offset, data value) = 0;

protected:
       const std::size_t m_size;
};

struct memory_port {
       MemoryInterfaceable* module;
       address base_address;
};

////////// ROM & RAM(s) : provides unified definition

template<std::size_t t_size>
class RAM : public MemoryInterfaceable {
public:
       RAM() : MemoryInterfaceable(t_size) {};
       virtual data  read(address offset) override {return m_internal[offset];};
       virtual void write(address offset, data value) override {m_internal[offset] = value;};
protected:
       std::array<data, t_size> m_internal;
};

template<std::size_t t_size, typename lambda>
class BBRAM : public RAM<t_size> {
public:
       BBRAM(lambda callback_destroyed) : RAM<t_size>(), callback(callback_destroyed) {};
       virtual ~BBRAM() {callback();};
protected:
       lambda callback;
};

template<std::size_t t_size>
class ROM : public MemoryInterfaceable {
public:
    ROM() : MemoryInterfaceable(t_size) {}

       virtual data  read(address offset) {return data[offset];};
               void write(address offset, data value) {throw std::logic_error("Cannot write to ROM");};
public:
       std::array<data, t_size> data;
};
