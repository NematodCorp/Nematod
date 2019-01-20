#pragma once

#include <cstdint>
#include <vector>

#include "interface/memory.hpp"

//////////

class AddressSpace : public Memory_interface {
public:
    void add_read_port (memory_port port)
    { add_port_impl(port, m_read_ports ); }
    void add_write_port(memory_port port)
    { add_port_impl(port, m_write_ports); }

    void add_port(memory_port port)
    { add_read_port(port); add_write_port(port); }

    void remove_port(MemoryInterfaceable* to_remove); // A single module can have multiple ports and bases, creating mirroring
    void remove_port(address addr);

    void clear();

    // Master ports : used by transaction masters to initiate one
    virtual data  read(address ptr) override;
    virtual data  poke(address ptr) override;
    virtual void write(address ptr, data val) override;

private:
    void add_port_impl(memory_port port, std::vector<memory_port>& port_list);
    memory_port* find_port(address addr, std::vector<memory_port>& port_list);

protected:
    std::vector<memory_port> m_read_ports;
    std::vector<memory_port> m_write_ports;
    data m_last_written_value = 0x00; // Used to emulate open-bus.
};
