#pragma once

#include <vector>

#include "interface/interrupts.hpp"

template class InterruptEmitter<NMI>;
template class InterruptEmitter<IRQ>;

template class InterruptBus<NMI>;
template class InterruptBus<IRQ>;
