#pragma once

#include <vector>

#include "interface/clock.hpp"

class MasterClock {
public:
       void add_clock(ClockReceiver* to_add);
       void remove_clock(ClockReceiver* to_remove);

       void tick();
private:
       std::vector<ClockReceiver *> m_clocks;
};
