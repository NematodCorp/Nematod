#pragma once
#include <numeric>

enum interrupt_kind {IRQ, NMI};

template<interrupt_kind T>
class InterruptEmitter {
public:
       bool is_raised() const {return m_state;};
protected:
       void raise_interrupt() {m_state = true;};
       void release_interrupt() {m_state = false;};
private:
       bool m_state = false;
};

// The two interrupt busses are NOT class related, as far as other modules are concerned

class NMIInterruptBus_interface {
public:
       virtual bool is_asserted() = 0;
};

class IRQInterruptBus_interface {
public:
       virtual bool is_asserted() = 0;
};
