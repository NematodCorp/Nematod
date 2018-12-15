#pragma once

#include "interface/clock.hpp"
#include "interface/interrupts.hpp"
#include "sequencer.hpp"


class FrameSequencer : public DividedClockReceiver<89490>, public InterruptEmitter<IRQ> {
public:
       void on_active_clock();
       int  on_read(int offset);
       void on_write(int offset, int data);

       int  get_state() {return state;};
       void clear_interrupt() {InterruptEmitter<IRQ>::release_interrupt();}
private:
       int state;

       bool is_mode1   = false;
       bool IRQ_dis = false;

       sequencer<0b001, 0b011, 0b001, 0b111>        mode0;
       sequencer<0b011, 0b001, 0b011, 0b001, 0b000> mode1;
};
