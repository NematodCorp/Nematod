#include <algorithm>
#include <cassert>

#include "common/log.hpp"

#include "memory.hpp"

void AddressSpace::add_port_impl(memory_port port, std::vector<memory_port> &port_list)
{
    // check pre-conditions
    assert(port.module != nullptr && "Adding a port whose module is a null pointer was attempted");

    for(std::size_t i(0); i < port.module->size(); ++i) {
        address to_check_free(i + port.base_address);
        auto checking_lambda = [=](const memory_port &a) {return a.base_address <= to_check_free &&
                    to_check_free < a.base_address + a.module->size();};

        if(std::any_of(port_list.begin(), port_list.end(), checking_lambda)) {
            assert(false && "Overlapping ports were tried to be placed");
        }
    }

    // port is good now
    port_list.push_back(port);

    std::sort(port_list.begin(), port_list.end(),
              [](const memory_port &a, const memory_port &b){return a.base_address < b.base_address;});
}

memory_port *AddressSpace::find_port(address addr, std::vector<memory_port> &port_list)
{
    // O(n) but actually much faster than a binary search due to a lower constant time
    auto it = port_list.begin();
    while (it->base_address <= addr && it != port_list.end())
        ++it;
    // The return it is *greater* than the address we search for.
    // If a mem modules hits, it must be the one before it.

    if(it == port_list.begin()) {
        return nullptr;
    }

    it--;

    if(it->module->valid() && addr < (it->base_address + it->module->size())) {

        return &(*it);
    } else {
        return nullptr; // Open bus (modules aren't countiguous in memory)
    }
}

void AddressSpace::remove_port(MemoryInterfaceable* to_remove) {
    m_read_ports.erase(std::remove_if(m_read_ports.begin(), m_read_ports.end(), [=](const memory_port& a){return a.module == to_remove;}),
                       m_read_ports.end());
    m_write_ports.erase(std::remove_if(m_write_ports.begin(), m_write_ports.end(), [=](const memory_port& a){return a.module == to_remove;}),
                        m_write_ports.end());

    // removing elements from a sorted container keeps it sorted
}

void AddressSpace::remove_port(address addr)
{
    m_read_ports.erase(std::remove_if(m_read_ports.begin(), m_read_ports.end(), [=](const memory_port& a){return a.base_address == addr;}),
                       m_read_ports.end());
    m_write_ports.erase(std::remove_if(m_write_ports.begin(), m_write_ports.end(), [=](const memory_port& a){return a.base_address == addr;}),
                        m_write_ports.end());

    // removing elements from a sorted container keeps it sorted
}

void AddressSpace::clear()
{
    m_read_ports.clear();
    m_write_ports.clear();
}

data AddressSpace::read(address ptr) {
    auto* port = find_port(ptr, m_read_ports);
    if (!port)
    {
        info("read open bus at 0x%x\n", ptr);

        return m_last_bus_value; // open bus
    }
    else
        return m_last_bus_value = port->module->read(ptr - port->base_address);
}

data AddressSpace::poke(address ptr)
{
    auto* port = find_port(ptr, m_read_ports);
    if (!port)
    {
        info("poke open bus at 0x%x\n", ptr);
        //assert(false);
        return m_last_bus_value; // open bus
    }
    else
        return port->module->poke(ptr - port->base_address);
}

void AddressSpace::write(address ptr, data value) {
    auto* port = find_port(ptr, m_write_ports);
    m_last_bus_value = value; // open bus

    if (!port)
    {
        info("write open bus at 0x%x\n", ptr);
        //assert(false);
    }
    else
        port->module->write(ptr - port->base_address, value);
}
