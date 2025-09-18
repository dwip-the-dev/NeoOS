#include "vga.h"
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t* vga_buffer = (uint16_t*)0xB8000;
static int vga_row = 0;
static int vga_col = 0;

void vga_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (uint16_t)' ' | (0x0F << 8); // White on black
    }
    vga_row = 0;
    vga_col = 0;
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else if (c == '\b') {
        if (vga_col > 0) vga_col--;
        vga_buffer[vga_row * VGA_WIDTH + vga_col] = (uint16_t)' ' | (0x0F << 8);
    } else {
        vga_buffer[vga_row * VGA_WIDTH + vga_col] = (uint16_t)c | (0x0F << 8);
        vga_col++;
    }
    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
    }
    if (vga_row >= VGA_HEIGHT) {
        // Scroll
        for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
            vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
        }
        for (int i = 0; i < VGA_WIDTH; i++) {
            vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + i] = (uint16_t)' ' | (0x0F << 8);
        }
        vga_row = VGA_HEIGHT - 1;
    }
}

void vga_print(const char* str) {
    for (int i = 0; str[i]; i++) vga_putchar(str[i]);
}

void vga_println(const char* str) {
    vga_print(str);
    vga_putchar('\n');
}
