#ifndef VGA_H
#define VGA_H

void vga_clear(void);
void vga_putchar(char c);
void vga_print(const char* s);
void vga_println(const char* s);
void vga_scroll(void);

#endif
