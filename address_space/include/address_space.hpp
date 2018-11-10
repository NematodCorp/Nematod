#pragma once

#include <cstdint>
#include <vector>

#include "interface/address_space.hpp"

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
