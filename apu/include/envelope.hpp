#pragma once

#include "divider.hpp"

struct envelope_gen {
	unsigned int result() {if((byte >> 4) & 1){return byte & 0xf;} else {return counter;}}

	void tick() {
		if(fourthreg_flag) {
			fourthreg_flag = false;
			counter = 0xf;
			div.reset();
		} else {
			div.tick();
		}

		if(div()) {
			if((byte >> 5) & 1 && (counter == 0)) {
				counter = 0xf;
			} else if(counter != 0) {
				counter--;
			}
		}
	}

	void write(data value) {byte = value; div.period = byte & 0xf;}
	void notice_fourth_reg() {fourthreg_flag = true;}
protected:
	unsigned int counter;

	data byte;
	bool fourthreg_flag;
	divider div;
}