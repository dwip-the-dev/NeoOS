#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
void poll_keyboard(void);
char getchar(void);
int has_key(void);

#endif
