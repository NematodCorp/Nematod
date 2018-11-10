#include <array>

#include "gtest/gtest.h"

#include "memory/include/memory.hpp"

TEST(Memory, AddRemovePort) {
       RAM<0x10000> ram;
       AddressSpace s;

       s.add_port(memory_port{&ram, 0x0});
       s.remove_port(&ram);
}

TEST(Memory, RemoveVoidPort) {
       AddressSpace s;
       s.remove_port(reinterpret_cast<MemoryInterfaceable *>(static_cast<intptr_t>(0xdeadbeef)));
}

TEST(Memory, PortSelection) {
       AddressSpace s;

       RAM<0x800> a1;
       RAM<0x800> a2;

       s.add_port(memory_port{&a1, 0x0000});
       s.add_port(memory_port{&a2, 0x0800});

       s.write(0x0000, 1);
       s.write(0x0800, 2);

       EXPECT_EQ(s.read(0x0000), a1.read(0));
       EXPECT_EQ(s.read(0x0800), a2.read(0));

       EXPECT_EQ(s.read(0x0000), 1);
       EXPECT_EQ(s.read(0x0800), 2);
}

TEST(Memory, OverlapException) {
       AddressSpace s;

       RAM<0x800>  a1;
       RAM<0x800>  a2;

       s.add_port(memory_port{&a1,     0});

       try {
              s.add_port(memory_port{&a2,  0xFF});
       } catch(std::exception &e) {
              return;
       }

       FAIL();
}

TEST(Memory, MultipleBases) {
       AddressSpace s;

       RAM<0x100> a;


       s.add_port(memory_port{&a, 0x000});
       s.add_port(memory_port{&a, 0x100});

       s.write(0      , 0xaa);
       s.write(0x101  , 0x55);

       EXPECT_EQ(s.read(0), a.read(0));
       EXPECT_EQ(s.read(1), a.read(1));
       EXPECT_EQ(s.read(0), 0xaa);
       EXPECT_EQ(s.read(1), 0x55);
}

TEST(Memory, OpenBus) {
       AddressSpace s;

       RAM<0x100> a;

       s.add_port(memory_port{&a, 0});

       s.write(0, 0xff);

       EXPECT_EQ(s.read(0xdead), 0xff);
}
