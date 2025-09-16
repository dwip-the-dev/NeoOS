#include <stdint.h>
#include <stddef.h>
#include "vga.h"
#include "keyboard.h"
#include "shell.h"

void kernel_main(void) {
    // Clear screen and boot banner
    vga_clear();
    vga_println("Booting NeoOS...");

    // Initialize hardware
    keyboard_init();

    // Run the shell loop
    shell_run();

    // Should never return
    while (1) {}
}
