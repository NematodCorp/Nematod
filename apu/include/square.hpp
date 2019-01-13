#pragma once
#include "frequency_sweep.hpp"
#include "divider.hpp"
#include "envelope.hpp"
#include "sequencer.hpp"
#include "length_counter.hpp"

template<SUT type>
struct SquareChannel {
	int result();
protected:
	envelope   envelop;
	sweep_unit sweepu;
	divider    timer;

	int mode;
	int step;

	constexpr static unsigned int sequencer[] {0, 1, 0, 0, 0, 0, 0, 0,
											   0, 1, 1, 0, 0, 0, 0, 0,
											   0, 1, 1, 1, 1, 0, 0, 0,
											   1, 0, 0, 1, 1, 1, 1, 1};
};