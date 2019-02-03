#pragma once

#include <cstdint>
#include <cassert>
#include <array>
#include <istream>

using address = std::uint16_t;
using data    = std::uint8_t;

class Memory_interface {
public:
    virtual data  read(address addr) = 0;
    // used to read data without side-effects
    virtual data  poke(address addr) { return read(addr); }
    virtual void write(address addr, data value) = 0;
};

////////// Attributes used for MMIO

class MemoryInterfaceable {
public:
    MemoryInterfaceable(std::size_t size) : m_size(size), m_valid(true) {};
    virtual ~MemoryInterfaceable() = default;

    std::size_t size() const {return m_size;};
    bool        valid() const{return m_valid;}
    void        set_valid(bool valid)
    { m_valid = valid; }

    virtual data  read(address offset)             = 0;
    // used to read data without side-effects
    virtual data  poke(address offset) { return read(offset); }
    virtual void write(address offset, data value) = 0;

protected:
    std::size_t m_size;
    bool        m_valid;
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
    virtual data  read(address offset) override {return m_data[offset];};
    virtual void write(address offset, data value) override {m_data[offset] = value;};
public:
    std::array<data, t_size> m_data;
};

template<std::size_t t_size, typename lambda> // Battery-backed RAM
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

    virtual data  read(address offset) {return m_data[offset];};
    void write(address, data)
    {
        //throw std::logic_error("Cannot write to ROM");
    };
public:
    std::array<data, t_size> m_data;
};

template <size_t t_size, bool writeable>
struct BankWindow : public MemoryInterfaceable
{
public:
    BankWindow() : MemoryInterfaceable(t_size) {}

    void set_rom_base(data* in_rom_base, size_t in_rom_size)
    {
        assert(in_rom_size % t_size == 0); // assert that rom_size is a multiple of bank_size

        rom_base = rom_ptr = in_rom_base;
        rom_bank_count = in_rom_size / t_size;
    }

    void set_bank(size_t bank_number)
    {
        bank_number %= rom_bank_count;

        cur_bank = bank_number;

        rom_ptr = rom_base + cur_bank*t_size;
    }

    unsigned bank() const
    { return cur_bank; }

    size_t bank_count() const
    { return rom_bank_count; }

    virtual data  read(address offset) {return rom_ptr[offset];};
    void write(address addr, data val)
    {
        if constexpr (writeable)
        {
            rom_ptr[addr] = val;
        }
    };
private:
    data* rom_base { nullptr };
    size_t cur_bank { 0 };
    size_t rom_bank_count { 0 };

    data* rom_ptr { nullptr };
};

template <size_t t_size>
using ROMBankWindow = BankWindow<t_size, false>;
template <size_t t_size>
using RAMBankWindow = BankWindow<t_size, true>;
