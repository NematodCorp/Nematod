#pragma once

#include "divider.hpp"

enum SUT {Pulse1, Pulse2};

template<SUT type>
struct sweep_unit {
	int get_result(int period) {if(negate) {if(type == Pulse1) {period - (period >> s) - 1} else {return period - (period >> s);}} 
								else {return period + (period >> s);}}

	bool muted(int curr_period) {return (get_result(curr_period) > 0x7FF) || (curr_period < 0x8);}
	void write(data value) {byte = value; written_flag = true;}

	void tick(int& curr_period) {
		div.tick();

		if(written_flag) { // Dubious
			written_flag = false;
			div.period = (byte >> 4) & 0x7;
			div.reset();
		}
	
		if((byte >> 7) && (byte & 0x7) && div() && !muted(curr_period)) {
			curr_period = get_result(curr_period);
		}
	}

protected:
	data byte;
	bool written_flag;

	divider div;
};