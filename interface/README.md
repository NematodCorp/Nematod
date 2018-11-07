# "Interface" headers

To provide clean interfaces, some libraries must define classes. Other modules
may use them. For example, a sound module may need memory access, and the controller of the module will be of a child type to
`MemoryInterfaceable`.  


However, each module should be a uncoupled and standalone as possible. To prevent modular interdependence,
each module can only include an "interface" headers. Using the previous example,
a sound controller **shouldn't** rely on particular capabilities from the address space.  


This interface system is weakly coupled. When trying to extract a specific module from the project, one
may copy the interface headers used by said modules. Again using the example, it makes perfect sense
that a sound controller has `read` and `write` methods, used to access its memory-mapped registers.
