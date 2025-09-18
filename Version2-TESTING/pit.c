#include "pit.h"
#include "io.h"  // Replaced inline outb with io.h
#include "vga.h"

volatile uint32_t pit_ticks = 0;
static uint32_t ticks_per_second = 0;

void pit_init(uint32_t hz) {
    ticks_per_second = hz;
    uint32_t divisor = 1193180 / hz; // PIT base frequency
    outb(0x43, 0x36); // Channel 0, lobyte/hibyte, rate generator
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

void pit_handler(void) {
    pit_ticks++;
    // Optional debug print (uncomment for testing)
    // vga_print("TICK ");
    if (pit_ticks % ticks_per_second == 0) {
        increment_time();
    }
}
