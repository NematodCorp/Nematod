#pragma once

#include <cstdint>
#include <vector>

#include "interface/memory.hpp"

//////////

class AddressSpace : public MemoryHandler {
public:
       void add_port(memory_port port);
       void remove_port(MemoryInterfaceable* to_remove); // A single module can have multiple ports and bases, creating mirroring

       // Master ports : used by transaction masters to initiate one
       virtual data  read(address ptr) override;
       virtual void write(address ptr, data val) override;

protected:
       std::vector<memory_port> m_ports;
       data m_last_written_value = 0xAA; // Used to emulate open-bus.
};
