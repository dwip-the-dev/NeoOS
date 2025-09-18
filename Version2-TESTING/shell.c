#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "commands.h"
#include "art.h"
#include "time.h"
#include "pit.h"

#define INPUT_BUFFER 128
#define VGA_WIDTH 80
#define MAX_HISTORY 16
#define PROMPT_LEN 11  // "HH:MM:SS-> "

// Simple strlen replacement
static int kstrlen(const char* s) {
    int len = 0;
    while (s[len] != '\0') len++;
    return len;
}

// Command history
static char history[MAX_HISTORY][INPUT_BUFFER];
static int history_count = 0;
static int history_pos = 0;

// Current input state
static char input[INPUT_BUFFER];
static int cursor_pos = 0;
static int input_length = 0;

// Print prompt with current time
static void shell_prompt(void) {
    char timebuf[9];
    time_to_string(timebuf);
    vga_print(timebuf);
    vga_print("-> ");
}

// Clear current input line and redraw it (full line clear for safety)
static void redraw_input(void) {
    // Back to column 0 (max back)
    for (int i = 0; i < VGA_WIDTH; i++) {
        vga_putchar('\b');
    }
    
    // Clear the entire line
    for (int i = 0; i < VGA_WIDTH; i++) {
        vga_putchar(' ');
    }
    
    // Back to column 0 again
    for (int i = 0; i < VGA_WIDTH; i++) {
        vga_putchar('\b');
    }
    
    // Print prompt and current input
    shell_prompt();
    for (int i = 0; i < input_length; i++) {
        vga_putchar(input[i]);
    }
    
    // Position cursor correctly (back from end of input)
    int move_back = input_length - cursor_pos;
    for (int i = 0; i < move_back; i++) {
        vga_putchar('\b');
    }
}

// Add command to history
static void add_to_history(const char* cmd) {
    if (cmd[0] == '\0') return; // Don't add empty commands
    
    // Shift history up if full
    if (history_count == MAX_HISTORY) {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            int j;
            for (j = 0; history[i+1][j] != '\0'; j++) {
                history[i][j] = history[i+1][j];
            }
            history[i][j] = '\0';
        }
        history_count--;
    }
    
    // Add new command to history
    int i;
    for (i = 0; cmd[i] != '\0' && i < INPUT_BUFFER - 1; i++) {
        history[history_count][i] = cmd[i];
    }
    history[history_count][i] = '\0';
    history_count++;
    history_pos = history_count;
}

// Handle special keys
static void handle_special_key(int key) {
    switch (key) {
        case KEY_UP:
            if (history_pos > 0) {
                history_pos--;
                // Copy history command to input
                int i;
                for (i = 0; history[history_pos][i] != '\0' && i < INPUT_BUFFER - 1; i++) {
                    input[i] = history[history_pos][i];
                }
                input_length = i;
                input[input_length] = '\0';
                cursor_pos = input_length;
                redraw_input();
            }
            break;
            
        case KEY_DOWN:
            if (history_pos < history_count - 1) {
                history_pos++;
                // Copy history command to input
                int i;
                for (i = 0; history[history_pos][i] != '\0' && i < INPUT_BUFFER - 1; i++) {
                    input[i] = history[history_pos][i];
                }
                input_length = i;
                input[input_length] = '\0';
                cursor_pos = input_length;
                redraw_input();
            } else if (history_pos == history_count - 1) {
                history_pos = history_count;
                // Clear input
                input_length = 0;
                input[0] = '\0';
                cursor_pos = 0;
                redraw_input();
            }
            break;
            
        case KEY_LEFT:
            if (cursor_pos > 0) {
                cursor_pos--;
                redraw_input();
            }
            break;
            
        case KEY_RIGHT:
            if (cursor_pos < input_length) {
                cursor_pos++;
                redraw_input();
            }
            break;
            
        case KEY_DELETE:
            if (cursor_pos < input_length) {
                // Shift all characters left
                for (int i = cursor_pos; i < input_length; i++) {
                    input[i] = input[i + 1];
                }
                input_length--;
                input[input_length] = '\0';  // Ensure null-terminated
                redraw_input();
            }
            break;
    }
}

void shell_run(void) {
    vga_clear();

    // Skip two lines to start logo at row 2
    vga_println("");
    vga_println("");

    // Print ASCII logo centered
    for (int i = 0; logo[i] != 0; i++) {
        int len = kstrlen(logo[i]);
        int spaces = (VGA_WIDTH - len) / 2;
        for (int s = 0; s < spaces; s++) {
            vga_putchar(' ');
        }
        vga_print(logo[i]);
        vga_putchar('\n');
    }

    vga_println("");  // Extra line after logo
    shell_prompt();

    static uint32_t last_update = 0;

    while (1) {
        // Live update prompt with new time when idle (input empty)
        if (input_length == 0 && pit_ticks != last_update) {
            redraw_input();
            last_update = pit_ticks;
        }

        // REMOVED: poll_keyboard(); - Using interrupts instead
        
        if (has_key()) {
            int key = getkey(); // Modified to return special keys as negative values
            
            if (key < 0) {
                // Special key handling
                handle_special_key(key);
            } else if (key == '\n') {
                input[input_length] = '\0';
                vga_putchar('\n');
                
                // Add to history and execute
                add_to_history(input);
                run_command(input);
                
                // Reset input state
                input_length = 0;
                cursor_pos = 0;
                input[0] = '\0';
                history_pos = history_count;
                
                shell_prompt();
            } else if (key == '\b') {
                // Backspace handling
                if (cursor_pos > 0) {
                    // Shift all characters left
                    for (int i = cursor_pos - 1; i < input_length; i++) {
                        input[i] = input[i + 1];
                    }
                    input_length--;
                    cursor_pos--;
                    input[input_length] = '\0';  // Ensure null-terminated
                    redraw_input();
                }
            } else {
                // Regular character input
                if (input_length < INPUT_BUFFER - 1) {
                    // Make space for new character
                    for (int i = input_length; i > cursor_pos; i--) {
                        input[i] = input[i - 1];
                    }
                    
                    // Insert character
                    input[cursor_pos] = key;
                    input_length++;
                    cursor_pos++;
                    input[input_length] = '\0';  // Ensure null-terminated
                    
                    redraw_input();
                }
            }
        }
        
        // Add a small delay to prevent CPU spinning
        __asm__ volatile("pause");
    }
}
