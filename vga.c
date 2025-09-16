#include "vga.h"
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t*)0xB8000)
#define WHITE_ON_BLACK 0x0F

static int cursor_x = 0;
static int cursor_y = 0;

// Clear screen
void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[y * VGA_WIDTH + x] = ((uint16_t)WHITE_ON_BLACK << 8) | ' ';
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

// Scroll screen up by one line
void vga_scroll(void) {
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[(y - 1) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
        }
    }
    // Clear last line
    for (int x = 0; x < VGA_WIDTH; x++) {
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = ((uint16_t)WHITE_ON_BLACK << 8) | ' ';
    }
    if (cursor_y > 0) cursor_y--;
}

// Print single character and update cursor
void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) vga_scroll();
        return;
    }
    if (c == '\b') {
        if (cursor_x > 0) cursor_x--;
        else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_WIDTH - 1;
        }
        VGA_MEMORY[cursor_y * VGA_WIDTH + cursor_x] = ((uint16_t)WHITE_ON_BLACK << 8) | ' ';
        return;
    }

    VGA_MEMORY[cursor_y * VGA_WIDTH + cursor_x] = ((uint16_t)WHITE_ON_BLACK << 8) | c;
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) vga_scroll();
    }
}

// Print string
void vga_print(const char* s) {
    for (int i = 0; s[i] != '\0'; i++) {
        vga_putchar(s[i]);
    }
}

// Print string with newline
void vga_println(const char* s) {
    vga_print(s);
    vga_putchar('\n');
}
