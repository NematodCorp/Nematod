#pragma once

#include <cstdint>
#include <vector>

using address = std::uint16_t;
using data    = std::uint8_t;

//////////

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

//////////

struct memory_port {
       MemoryInterfaceable* module;
       address base_address;
};

class AddressSpace {
public:
       void add_port(memory_port port); // Avoiding "easy" ownership conflicts
       void remove_port(MemoryInterfaceable* to_remove); // A single module can have multiple ports and bases, creating mirroring

       // Master ports : used by transaction masters to initiate one
       data  read(address ptr);
       void write(address ptr, data val);

protected:
       std::vector<memory_port> m_ports;
       data m_last_written_value = 0xAA; // Used to emulate open-bus.
};
