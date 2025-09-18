#include <stdint.h>
#include <stddef.h>
#include "vga.h"
#include "keyboard.h"
#include "shell.h"
#include "time.h"
#include "io.h"
#include "pit.h"
#include "idt.h"

// Convert integer to string (basic itoa for exceptions)
static void int_to_str(uint32_t n, char *str) {
    char buf[12]; // enough for 32-bit int
    int i = 0;
    if (n == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    // reverse into str
    int j = 0;
    while (i > 0) {
        str[j++] = buf[--i];
    }
    str[j] = '\0';
}

// Exception handler
void exception_handler(uint32_t vector) {
    char buf[12];
    int_to_str(vector, buf);

    vga_print("Exception ");
    vga_print(buf);
    vga_println(" occurred! Halting...");

    __asm__ volatile("cli; hlt");
}

// Kernel entry point
void kernel_main(void) {
    vga_clear();
    vga_println("Booting NeoOS...");
    vga_println("Kernel loaded at 0x100000");

    // Initialize interrupts & PIC
    idt_init();
    vga_println("IDT & PIC initialized.");

    // Enable interrupts
    __asm__ volatile("sti");
    vga_println("Interrupts enabled.");

    // Keyboard
    keyboard_init();
    vga_println("Keyboard ready.");

    // Time
    set_time(0, 0, 0);
    vga_println("Time set to 00:00:00");

    // PIT
    pit_init(100);  // 100 Hz is better for responsiveness
    vga_println("PIT initialized at 100 Hz.");

    // Run shell
    vga_println("Starting shell...");
    shell_run();

    // Should never return
    vga_println("ERROR: shell returned!");
    __asm__ volatile("cli; hlt");
}
