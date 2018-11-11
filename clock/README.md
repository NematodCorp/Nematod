# Clock module
The clock module is here to provide a synchronized clock, available to all other
modules easily.

## Module's interface
The clock module makes available two main classes in its interface header :

```cpp
class ClockReceiver;
template<unsigned int ratio> class DividedClockReceiver;
```
They are designed to be inherited from by the classes who need clocks.  
Inheriting from `ClockReceiver` requires an implementation of `void tick()`.
This function is called whenever the clock ticks.  


`DividedClockReceiver` must be inherited from with a ratio. This ratio determines
the frequency of received ticks, relative to the master clock of the application.
`void tick()` is no longer available, replaced then by `void on_active_clock()`.

## Master's interface
To the already available capabilities, including the master header defines a class :

```cpp
class MasterClock {
public:
       void add_clock(ClockReceiver* to_add);
       void remove_clock(ClockReceiver* to_remove);

       void tick();
};
```

`add_clock` and `remove_clock` allow the polymorphic adding of clock receivers.
Calling `tick()` propagates it to all clock receivers.
