#pragma once

#include <vector>

class ClockReceiver {
public:
    virtual void tick() = 0;
};

class DividedClockReceiverInterface : public ClockReceiver
{
public:
    virtual void on_active_clock() = 0;
};

template<unsigned int ratio>
class DividedClockReceiver : public DividedClockReceiverInterface {
public:
    static constexpr unsigned int clock_rate = ratio;

    void tick() override {
        m_counter = (m_counter + 1) % ratio;
        if(m_counter == 0) on_active_clock();
    };
protected:
    unsigned int m_counter = 0;
};

class MasterClock {
public:
    void add_clock(ClockReceiver* to_add);
    void remove_clock(ClockReceiver* to_remove);

    void tick();
private:
    std::vector<ClockReceiver *> m_clocks;
};
