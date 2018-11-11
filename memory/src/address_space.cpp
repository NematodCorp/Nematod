#include <algorithm>
#include <stdexcept>

#include "memory.hpp"

void AddressSpace::add_port(memory_port port) {
       if(port.module == nullptr) throw std::logic_error("Adding a port whose module is a null pointer was attempted");

       for(std::size_t i(0); i < port.module->size(); ++i) {
              address to_check_free(i + port.base_address);
              auto checking_lambda = [=](const memory_port &a) {return a.base_address <= to_check_free &&
                                                                       to_check_free < a.base_address + a.module->size();};

              if(std::any_of(m_ports.begin(), m_ports.end(), checking_lambda)) {
                     throw std::runtime_error("Overlapping ports were tried to be placed");
              }
       }

       // port is good now
       m_ports.push_back(port);

       std::sort(m_ports.begin(), m_ports.end(),
                [](const memory_port &a, const memory_port &b){return a.base_address < b.base_address;});
}

void AddressSpace::remove_port(MemoryInterfaceable* to_remove) {
       m_ports.erase(std::remove_if(m_ports.begin(), m_ports.end(), [=](const memory_port& a){return a.module == to_remove;}),
                     m_ports.end());

       std::sort(m_ports.begin(), m_ports.end(),
                [](const memory_port &a, const memory_port &b){return a.base_address < b.base_address;});
}

data AddressSpace::read(address ptr) {
       auto comp = [](const memory_port &a, const memory_port &b) {return a.base_address < b.base_address;};
       auto it = std::upper_bound(m_ports.begin(), m_ports.end(), memory_port{nullptr, ptr}, comp);

       if(it == m_ports.end()) { // Open bus hit
              return m_last_written_value;
       } else {
              // The return it is *greater* than the address we search for.
              // If a mem modules hits, it must be the one before it.

              if(it == m_ports.begin()) {
                     return m_last_written_value; // Low address open bus special case
              }

              it--;

              if(ptr < (it->base_address + it->module->size())) {
                     return it->module->read(ptr - it->base_address); // Relative address
              } else {
                     return m_last_written_value; // Open bus (modules aren't countiguous in memory)
              }
       }
}

void AddressSpace::write(address ptr, data value) {
       auto checking_lambda = [=](const memory_port& a){return a.base_address <= ptr && ptr < a.base_address + a.module->size();};
       const auto it = std::find_if(m_ports.begin(), m_ports.end(), checking_lambda);

       if(it != m_ports.end()) {
              it->module->write(ptr - it->base_address, value);
       }

       m_last_written_value = value;
}
