# Memory module
As the name implies, this module provides general emulated memory handling across
other modules.

## Module's interface
This module's interface defines quite a bit of classes.  


- Firstly, it defines a `Memory_interface` abstract class. It offers the basic
`read` and `write` methods, without anything else.  


- Secondly, it defines a `MemoryInterfaceable` attribute class. Classes that
inherit from `MemoryInterfaceable` are eligible to management by `Memory_interface`'s
descendants. Note that to be `MemoryInterfaceable`, a child class must pass its memory size to `MemoryInterfaceable` constructor.

- Thirdly, it defines the `memory_port` structure. It describes a `MemoryInterfaceable` with a base address.
It is used internally, and by modules whose base address is dynamically and thus unknown to the master.

- Finally, it defines the typedefs of `address` and `data`, as well as some minor helper classes, such as `RAM` and `ROM`.

## Master's interface
The master can use, additionally from the previously described interface, the `AddressSpace` class, whose
interface is the self-explanatory following :

```cpp
void add_port(memory_port port);
void remove_port(MemoryInterfaceable* to_remove);

virtual data  read(address ptr);
virtual void write(address ptr, data val);
```

`AddressSpace` multiplexes the request issued by `read` and `write` to the different
`MemoryInterfaceable` objects added or removed by `add_port` and `remove_port`. It also handles the NES open bus behavior
Two ports must not overlap, but can have the same `MemoryInterfaceable` pointer, in which
case the two ports are mirrors of each other.
