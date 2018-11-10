#pragma once

#include <vector>

#include "interface/interrupts.hpp"


// The default InterruptBus is a Common Emitter bus.
template<interrupt_kind T>
class InterruptBus : public InterruptManager<T> {
public:
       void add_slave(InterruptEmitter<T> *to_add);
       void remove_slave(InterruptEmitter<T> *to_remove);

       virtual bool is_asserted() override;

protected:
       virtual bool eval_method(bool curr_state, const InterruptEmitter<T>& take_in_account);
       // If state is "true" after having taken everything into account => exception raised

       std::vector<InterruptEmitter<T> *> m_ie;
};


template class InterruptEmitter<NMI>;
template class InterruptEmitter<IRQ>;

template class InterruptBus<NMI>;
template class InterruptBus<IRQ>;
