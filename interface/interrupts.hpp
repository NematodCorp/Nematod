#pragma once
#include <numeric>

enum interrupt_kind {IRQ, NMI};


template<interrupt_kind T>
class InterruptEmitter {
public:
       bool is_raised() {if constexpr(T == IRQ){return m_state;}
                         else {auto ret = m_state; m_state = false; return ret;}};
protected:
       void raise_interrupt() {m_state = true;};
       void release_interrupt() {m_state = false;};
private:
       bool m_state = false;
};

// The default InterruptBus is a Common Emitter bus.
template<interrupt_kind T>
class InterruptBus {
public:
       void add_slave(InterruptEmitter<T> *to_add);
       void remove_slave(InterruptEmitter<T> *to_remove);

       virtual bool is_asserted();

protected:
       virtual bool eval_method(bool curr_state, InterruptEmitter<T>& take_in_account);
       // If state is "true" after having taken everything into account => exception raised

       std::vector<InterruptEmitter<T> *> m_ie;
};
