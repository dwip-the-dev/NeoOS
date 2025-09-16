#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "commands.h"
#include "art.h"

#define INPUT_BUFFER 128
#define VGA_WIDTH 80

// Simple strlen replacement
static int kstrlen(const char* s) {
    int len = 0;
    while (s[len] != '\0') len++;
    return len;
}

// Print prompt
static void shell_prompt(void) {
    vga_print("-> ");
}

void shell_run(void) {
    char input[INPUT_BUFFER];
    int pos = 0;

    vga_clear();

    // Print ASCII logo centered
    for (int i = 0; logo[i] != 0; i++) {
        int x = (VGA_WIDTH - kstrlen(logo[i])) / 2;
        int y = i + 2;
        for (int j = 0; logo[i][j] != '\0'; j++) {
            vga_putchar(logo[i][j]);
        }
        vga_putchar('\n');
    }

    shell_prompt();

    while (1) {
        poll_keyboard();
        if (has_key()) {
            char c = getchar();

            if (c == '\n') {
                input[pos] = '\0';
                vga_putchar('\n');
                run_command(input); // prints output using vga_println internally
                shell_prompt();     // new prompt after output
                pos = 0;
            }
            else if (c == '\b') {
                if (pos > 0) {
                    pos--;
                    vga_putchar('\b'); // backspace handled in vga.c
                }
            }
            else {
                if (pos < INPUT_BUFFER - 1) {
                    input[pos++] = c;
                    vga_putchar(c); // echo character
                }
            }
        }
    }
}
