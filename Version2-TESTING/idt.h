#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_ENTRIES 256

// IDT entry structure
struct idt_entry {
    uint16_t base_low;   // Lower 16 bits of handler address
    uint16_t sel;        // Kernel segment selector
    uint8_t  always0;    // Must be zero
    uint8_t  flags;      // Flags
    uint16_t base_high;  // Upper 16 bits of handler address
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// IDT table and pointer (defined in idt.c)
extern struct idt_entry idt[IDT_ENTRIES];
extern struct idt_ptr idtp;

// Functions
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void idt_init(void);

#endif // IDT_H
