#include <algorithm>
#include <stdexcept>

#include "interrupts.hpp"

template<interrupt_kind T>
void InterruptBus<T>::add_slave(InterruptEmitter<T> *to_add) {
       if(to_add == nullptr) std::logic_error("Cannot add NULL as an interrupt emitter !");

       if(std::find(m_ie.begin(), m_ie.end(), to_add) == m_ie.end()) {
              m_ie.push_back(to_add);
       } else {
              std::logic_error("Cannot doubly add an interrupt emitter !");
       }
}

template<interrupt_kind T>
void InterruptBus<T>::remove_slave(InterruptEmitter<T> *to_remove) {
       m_ie.erase(std::remove(m_ie.begin(), m_ie.end(), to_remove), m_ie.end());
}

template<interrupt_kind T>
bool InterruptBus<T>::is_asserted() {
       bool asserted(false);
       for(auto a : m_ie) {
              asserted = eval_method(asserted, *a);
       }
       return asserted;
}

template<interrupt_kind T>
bool InterruptBus<T>::eval_method(bool curr_state, const InterruptEmitter<T>& take_in_account) {
       return curr_state | take_in_account.is_raised();
}
