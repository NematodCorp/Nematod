# Interrupt module
The interrupt module provides two kinds of interrupt buses : an IRQ, and
an NMI bus, triggered respectively by level or edge.

## Module's interface
The interrupt module defines two simple interfaces to handle interrupt buses for
other modules :

```cpp
class NMIInterruptBus_interface;
class IRQInterruptBus_interface;
```
For dissociating the control block with their signaling function, they are
virtual classes and should be referenced by pointers.  
They each provide a public `bool is_asserted()` method. If `is_asserted()` happens to
be `true`, then interrupt action must be carried by the reader of the object.

It also defines two kinds of interrupt emitters which must be inherited
from to be able to send interrupt, `InterruptEmitter<NMI>` and `InterruptEmitter<IRQ>`,
using the following self-describing template interface :

```cpp
template<interrupt_kind T>
class InterruptEmitter {
public:
       bool is_raised() const;
protected:
       void raise_interrupt();
       void release_interrupt();
```

As seen, `interrupt_kind` is an enumeration containing the two interrupt kinds,
NMI and IRQ.

## Master's interface
The master's interface of the interrupt module mainly defines :

```cpp
class NMIInterruptBus;
class IRQInterruptBus;
```

These classes define the purely virtual methods of the respective `*_interface`
class and include a control block. The control block's methods are :

```cpp
template<interrupt_kind T>
InterruptBus_CtrlBlock<T>::add_slave(InterruptEmitter<T> *to_add);

template<interrupt_kind T>
InterruptBus_CtrlBlock<T>::remove_slave(InterruptEmitter<T> *to_remove);
```

Which add or remove a slave device from the bus. It can be seen that the
control block is a template class. This eases implementation, but also ensures that
the `NMI*` and `IRQ*` cannot be polymorphed into a single type, for they should be distinct in their handling.
