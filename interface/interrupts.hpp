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

template<interrupt_kind T>
class InterruptManager {
public:
       virtual bool is_asserted() = 0;
};
