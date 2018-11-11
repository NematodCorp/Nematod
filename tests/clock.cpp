#include "gtest/gtest.h"

#include "clock/include/clock.hpp"

TEST(Clock, AddRemoveReceiver) {
       MasterClock s;

       struct ClockCounter : public ClockReceiver {
              void tick() override {counter++;};
              unsigned int counter = 0;
       };

       ClockCounter a;

       s.add_clock(&a);
       s.remove_clock(&a);
}

TEST(Clock, RemoveVoidReceiver) {
       MasterClock s;
       s.remove_clock(reinterpret_cast<ClockReceiver *>(static_cast<intptr_t>(0xc5ed1b1e)));
}

TEST(Clock, Ticking) {
       MasterClock s;

       struct ClockCounter : public ClockReceiver {
              void tick() override {counter++;};
              unsigned int counter = 0;
       };

       ClockCounter a;

       s.add_clock(&a);

       EXPECT_EQ(a.counter, 0);
       s.tick();
       EXPECT_EQ(a.counter, 1);
}

template<unsigned int R>
struct ClockCounter : public DividedClockReceiver<R> {
       void on_active_clock() override {counter++;};
       unsigned int counter = 0;
};

TEST(Clock, SynchroneousTicking) {
       MasterClock s;

       ClockCounter<1> a;
       ClockCounter<2> b;
       ClockCounter<4> c;

       s.add_clock(&a);
       s.add_clock(&b);
       s.add_clock(&c);

       for(int i(0); i < 4; ++i) {
              s.tick();
       }

       EXPECT_EQ(a.counter, 4);
       EXPECT_EQ(b.counter, 2);
       EXPECT_EQ(c.counter, 1);
       EXPECT_EQ(a.counter / 2, b.counter);
       EXPECT_EQ(a.counter / 4, c.counter);
}
