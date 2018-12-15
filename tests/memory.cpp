#include <array>

#include "gtest/gtest.h"

#include "memory/include/memory.hpp"

namespace
{

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

    for (size_t i { 0 }; i < 0x800; ++i)
    {
        s.write(0x0000 + i, (i + 1) & 0xFF);
    }
    for (size_t i { 0 }; i < 0x800; ++i)
    {
        s.write(0x0800 + i, (i + 2) & 0xFF);
    }

    for (size_t i { 0 }; i < 0x800; ++i)
    {
        EXPECT_EQ(s.read(0x0000 + i), (i + 1) & 0xFF);
        EXPECT_EQ(s.read(0x0000 + i), a1.read(i));

        EXPECT_EQ(s.read(0x0800 + i), (i + 2) & 0xFF);
        EXPECT_EQ(s.read(0x0800 + i), a2.read(i));
    }

    a1.write(0, 1);
    a2.write(0, 2);

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
    RAM<0x100> b;

    s.add_port(memory_port{&a, 0xA});
    s.add_port(memory_port{&b, 0xf00A});

    s.write(2, 0xff);

    EXPECT_EQ(s.read(0xdead), 0xff);
    EXPECT_EQ(s.read(0), 0xff);
}

}
