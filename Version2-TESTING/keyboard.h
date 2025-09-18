#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

// Special key codes
#define KEY_UP -1
#define KEY_DOWN -2
#define KEY_LEFT -3
#define KEY_RIGHT -4
#define KEY_DELETE -5

// Function declarations
void keyboard_init(void);
void poll_keyboard(void);
int has_key(void);
int getkey(void);
char scancode_to_char(uint8_t scancode, int shift);
void keyboard_handler(void);  // ADD THIS

#endif
