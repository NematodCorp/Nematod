#include "frame_sequencer.hpp"
#include "gendefs.hpp"

int FrameSequencer::on_read(int) {
       throw illegal_read();
       return -0xDEADBEEF;
}

void FrameSequencer::on_write(int, int data) {
       DividedClockReceiver<89490>::m_counter = 0; // Reset divider
       mode0.reset();
       mode1.reset();

       IRQ_dis = (data >> 6) & 0x1;
       is_mode1   = (data >> 7) & 0x1;

       if(is_mode1) mode1.next();

       if(((state >> 2) & 0x1) && !IRQ_dis) {
              InterruptEmitter<IRQ>::raise_interrupt();
       } else {
              InterruptEmitter<IRQ>::release_interrupt();
       }
}

void FrameSequencer::on_active_clock() {
       int what_to_do = 0;

       if(is_mode1) {
              what_to_do = mode1.next();
       } else {
              what_to_do = mode0.next();
       }

       state = what_to_do;

       if(((state >> 2) & 0x1) && !IRQ_dis) {
              InterruptEmitter<IRQ>::raise_interrupt();
       } else {
              InterruptEmitter<IRQ>::release_interrupt();
       }
}
