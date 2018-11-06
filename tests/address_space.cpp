#include <array>

#include "gtest/gtest.h"

#include "address_space/include/address_space.hpp"

TEST(AddressSpace, AddRemovePort) {
       struct RAM : public MemoryInterfaceable {
              RAM() : MemoryInterfaceable(0x1000) {}
              virtual ~RAM() = default;

              data  read(address ptr)             override {return arr[ptr];};
              void write(address ptr, data value) override {arr[ptr] = value;};

              std::array<data, 0x1000> arr;
       };

       RAM ram;
       AddressSpace s;

       s.add_port(memory_port{&ram, 0x0});
       s.remove_port(&ram);
}

TEST(AddressSpace, RemoveVoidPort) {
       AddressSpace s;
       s.remove_port(reinterpret_cast<MemoryInterfaceable *>(static_cast<intptr_t>(0xdeadbeef)));
}

TEST(AddressSpace, PortSelection) {
       AddressSpace s;

       struct RAM : public MemoryInterfaceable {
              RAM() : MemoryInterfaceable(0x0800) {}
              virtual ~RAM() = default;

              data  read(address ptr)             override {return arr[ptr];};
              void write(address ptr, data value) override {arr[ptr] = value;};

              std::array<data, 0x0800> arr;
       };

       RAM a1;
       RAM a2;

       s.add_port(memory_port{&a1, 0x0000});
       s.add_port(memory_port{&a2, 0x0800});

       s.write(0x0000, 1);
       s.write(0x0800, 2);

       EXPECT_EQ(s.read(0x0000), a1.arr[0]);
       EXPECT_EQ(s.read(0x0800), a2.arr[0]);

       EXPECT_EQ(s.read(0x0000), 1);
       EXPECT_EQ(s.read(0x0800), 2);
}

TEST(AddressSpace, OverlapException) {
       AddressSpace s;

       struct RAM : public MemoryInterfaceable {
              RAM() : MemoryInterfaceable(0x0800) {}
              virtual ~RAM() = default;

              data  read(address ptr)             override {return arr[ptr];};
              void write(address ptr, data value) override {arr[ptr] = value;};

              std::array<data, 0x0800> arr;
       };

       RAM  a1;
       RAM  a2;

       s.add_port(memory_port{&a1,     0});

       try {
              s.add_port(memory_port{&a2,  0xFF});
       } catch(std::exception &e) {
              return;
       }

       FAIL();
}

TEST(AddressSpace, MultipleBases) {
       AddressSpace s;

       struct RAM : public MemoryInterfaceable {
              RAM() : MemoryInterfaceable(0x100) {}
              virtual ~RAM() = default;

              data  read(address ptr)             override {return arr[ptr];};
              void write(address ptr, data value) override {arr[ptr] = value;};

              std::array<data, 0x100> arr;
       };

       RAM a;


       s.add_port(memory_port{&a, 0x000});
       s.add_port(memory_port{&a, 0x100});

       s.write(0      , 0xaa);
       s.write(256 + 1, 0x55);

       EXPECT_EQ(s.read(0), a.arr[0]);
       EXPECT_EQ(s.read(1), a.arr[1]);
       EXPECT_EQ(s.read(0), 0xaa);
       EXPECT_EQ(s.read(1), 0x55);
}

TEST(AddressSpace, OpenBus) {
       AddressSpace s;

       struct RAM : public MemoryInterfaceable {
              RAM() : MemoryInterfaceable(0x100) {}
              virtual ~RAM() = default;

              data  read(address ptr)             override {return arr[ptr];};
              void write(address ptr, data value) override {arr[ptr] = value;};

              std::array<data, 0x100> arr;
       };

       RAM a;

       s.add_port(memory_port{&a, 0});

       s.write(0, 0xff);

       EXPECT_EQ(s.read(0xdead), 0xff);
}
