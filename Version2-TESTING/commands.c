#include "commands.h"
#include "vga.h"
#include "art.h"
#include "time.h"
#include <stdint.h>

// Minimal strcmp implementation
int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

// Minimal strncmp implementation
int strncmp(const char* a, const char* b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) return a[i] - b[i];
        if (a[i] == '\0') return 0;
    }
    return 0;
}

// Forward declaration (defined in time.c)
extern int parse_time_str(const char* str, uint8_t* hh, uint8_t* mm, uint8_t* ss);


// Time command
void time_command(char* args) {
    char time_str[9];

    if (args[0] != '\0') {
        uint8_t h, m, s;
        if (parse_time_str(args, &h, &m, &s)) {
            set_time(h, m, s);
            vga_print("Time set to: ");
        } else {
            vga_print("Invalid time format. Use HH:MM:SS, HH/MM/SS, or HH.MM.SS\n");
            return;
        }
    }

    time_to_string(time_str);
    vga_print(time_str);
    vga_print("\n");
}

// Run command function
void run_command(const char* cmd) {
    if (cmd[0] == '\0') return;

    // Time command
    if (strncmp(cmd, "time", 4) == 0) {
        if (cmd[4] == ' ' || cmd[4] == '\0') {
            time_command((char*)(cmd + 5)); // cast to remove const
        }
        return;
    }
    // Help command
    else if (strcmp(cmd, "help") == 0) {
        vga_println("Commands: help, clear, about, time");
    }
    // Clear screen
    else if (strcmp(cmd, "clear") == 0) {
        vga_clear();
    }
    // About info
    else if (strcmp(cmd, "about") == 0) {
        for (int i = 0; logo[i] != 0; i++) {
            vga_println(logo[i]);
        }
        vga_println("NeoOS v0.1 - Baremetal toy OS by Dwip");
    }
    // Unknown command
    else {
        vga_print("Unknown command: ");
        vga_println(cmd);
    }
}
