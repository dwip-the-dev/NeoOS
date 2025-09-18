#ifndef PIT_H
#define PIT_H

#include <stdint.h>
#include "time.h"

// Global tick count
extern volatile uint32_t pit_ticks;

// Initialize PIT to generate periodic interrupts at given Hz
void pit_init(uint32_t hz);

// PIT IRQ handler, called from IRQ0
void pit_handler(void);

#endif // PIT_H
