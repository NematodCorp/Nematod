#pragma once

#include <vector>

#include "interface/interrupts.hpp"


// The default InterruptBus is a Common Emitter bus.
template<interrupt_kind T>
class InterruptBus_CtrlBlock {
public:
       void add_slave(InterruptEmitter<T> *to_add);
       void remove_slave(InterruptEmitter<T> *to_remove);

protected:
       std::vector<InterruptEmitter<T> *> m_ie;
};

class NMIInterruptBus : public NMIInterruptBus_interface, public InterruptBus_CtrlBlock<NMI> {
public:
       virtual bool is_asserted() override;
protected:
       bool previous_state = false;
};

class IRQInterruptBus : public IRQInterruptBus_interface, public InterruptBus_CtrlBlock<IRQ> {
public:
       virtual bool is_asserted() override;
};

template class InterruptBus_CtrlBlock<NMI>;
template class InterruptBus_CtrlBlock<IRQ>;

template class InterruptEmitter<NMI>;
template class InterruptEmitter<IRQ>;
