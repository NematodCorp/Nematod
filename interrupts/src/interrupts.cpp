#include <algorithm>
#include <stdexcept>

#include "interrupts.hpp"

template<interrupt_kind T>
void InterruptBus_CtrlBlock<T>::add_slave(InterruptEmitter<T> *to_add) {
       if(to_add == nullptr) std::logic_error("Cannot add NULL as an interrupt emitter !");

       if(std::find(m_ie.begin(), m_ie.end(), to_add) == m_ie.end()) {
              m_ie.push_back(to_add);
       } else {
              std::logic_error("Cannot doubly add an interrupt emitter !");
       }
}

template<interrupt_kind T>
void InterruptBus_CtrlBlock<T>::remove_slave(InterruptEmitter<T> *to_remove) {
       m_ie.erase(std::remove(m_ie.begin(), m_ie.end(), to_remove), m_ie.end());
}

bool NMIInterruptBus::is_asserted() {
       bool bus_state = false;
       for(auto i : m_ie) {
              bus_state |= i->is_raised();
       }

       bool retval = bus_state & !previous_state;
       previous_state = bus_state;

       return retval;
}

bool IRQInterruptBus::is_asserted() {
       bool bus_state = false;
       for(auto i : m_ie) {
              bus_state |= i->is_raised();
       }

       return bus_state;
}

template class InterruptBus_CtrlBlock<NMI>;
template class InterruptBus_CtrlBlock<IRQ>;

template class InterruptEmitter<NMI>;
template class InterruptEmitter<IRQ>;
