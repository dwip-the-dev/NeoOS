#include "commands.h"
#include "vga.h"
#include "art.h"

void run_command(const char* cmd) {
    if (cmd[0] == '\0') {
        return; // ignore empty input
    }

    if (strcmp(cmd, "help") == 0) {
        vga_println("Commands: help, clear, about");
    } 
    else if (strcmp(cmd, "clear") == 0) {
        vga_clear();
    } 
    else if (strcmp(cmd, "about") == 0) {
        for (int i = 0; logo[i] != 0; i++) {
            vga_println(logo[i]);
        }
        vga_println("NesOS v0.1 - Baremetal OS by Dwip");
    } 
    else {
        vga_print("Unknown command: ");
        vga_println(cmd);
    }
}

// Minimal strcmp since libc isn’t available
int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}
