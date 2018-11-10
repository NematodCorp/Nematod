#include "gtest/gtest.h"

#include "interrupts/include/interrupts.hpp"

template<interrupt_kind T>
class ExposedEmitter : public InterruptEmitter<T> {
public:
       void e_raise()   {InterruptEmitter<T>::raise_interrupt();}
       void e_release() {InterruptEmitter<T>::release_interrupt();}

};

TEST(Interrupts, AddRemoveSlave) {
       InterruptEmitter<IRQ> ie;
       InterruptBus<IRQ> ib;

       ib.add_slave(&ie);
       ib.remove_slave(&ie);
}

TEST(Interrupts, RemoveVoidSlave) {
       InterruptBus<IRQ> ib;
       ib.remove_slave(reinterpret_cast<InterruptEmitter<IRQ> *>(static_cast<uintptr_t>(0xdeadbeef)));
}

TEST(Interrupts, SingleRaiseInterrupt) {
       InterruptBus<IRQ> ib;
       ExposedEmitter<IRQ> ee;

       ib.add_slave(&ee);

       EXPECT_FALSE(ib.is_asserted());
       ee.e_raise();
       EXPECT_TRUE(ib.is_asserted());

       ib.remove_slave(&ee);
}

TEST(Interrupts, SingleRaiseReleaseInterrupt) {
       InterruptBus<IRQ> ib;
       ExposedEmitter<IRQ> ee;

       ib.add_slave(&ee);

       EXPECT_FALSE(ib.is_asserted());
       ee.e_raise();
       EXPECT_TRUE(ib.is_asserted());
       ee.e_release();
       EXPECT_FALSE(ib.is_asserted());

       ib.remove_slave(&ee);
}

TEST(Interrupts, DoubleRaiseInterrupt) {
       InterruptBus<IRQ> ib;
       ExposedEmitter<IRQ> ee_a;
       ExposedEmitter<IRQ> ee_b;

       ib.add_slave(&ee_a);
       ib.add_slave(&ee_b);

       EXPECT_FALSE(ib.is_asserted());
       ee_a.e_raise();
       EXPECT_TRUE(ib.is_asserted());
       ee_b.e_raise();
       EXPECT_TRUE(ib.is_asserted());
}

TEST(Interrupts, DoubleRaiseReleaseInterrupt) {
       InterruptBus<IRQ> ib;
       ExposedEmitter<IRQ> ee_a;
       ExposedEmitter<IRQ> ee_b;

       ib.add_slave(&ee_a);
       ib.add_slave(&ee_b);

       EXPECT_FALSE(ib.is_asserted());
       ee_a.e_raise();
       EXPECT_TRUE(ib.is_asserted());
       ee_b.e_raise();
       EXPECT_TRUE(ib.is_asserted());
       ee_a.e_release();
       EXPECT_TRUE(ib.is_asserted());
       ee_b.e_release();
       EXPECT_FALSE(ib.is_asserted());
}
