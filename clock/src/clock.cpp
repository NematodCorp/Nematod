#include <algorithm>
#include <stdexcept>

#include "clock.hpp"

void MasterClock::add_clock(ClockReceiver* to_add) {
       if(to_add == nullptr) throw std::logic_error("Registering a clock whose receiver is a null pointer was attempted");

       if(std::find(m_clocks.begin(), m_clocks.end(), to_add) == m_clocks.end()) {
              m_clocks.push_back(to_add);
       } else {
              throw std::runtime_error("No receiver can register more than one instance");
       }
}

void MasterClock::remove_clock(ClockReceiver* to_remove) {
       m_clocks.erase(std::remove(m_clocks.begin(), m_clocks.end(), to_remove), m_clocks.end());
}

void MasterClock::tick() {
       for(auto i : m_clocks) {
              i->tick();
       }
}
