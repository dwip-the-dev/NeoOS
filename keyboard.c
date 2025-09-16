#include "keyboard.h"

#define KEYBOARD_DATA_PORT   0x60
#define KEYBOARD_STATUS_PORT 0x64
#define BUFFER_SIZE 256

static volatile char key_buffer[BUFFER_SIZE];
static volatile uint8_t head = 0;
static volatile uint8_t tail = 0;
static volatile int has_key_press = 0;

// US keyboard map (partial)
static const char keyboard_map[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
    'z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',' ',
    0,0,0,0,0,0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+',
    '1','2','3','0','.','0',0,0,0,0
};

// Low-level I/O
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Initialize keyboard (polling only, no interrupts)
void keyboard_init(void) {
    // nothing to do for polling
}

// Poll keyboard and store keys
void poll_keyboard(void) {
    uint8_t status = inb(KEYBOARD_STATUS_PORT);
    if (status & 0x01) {
        uint8_t code = inb(KEYBOARD_DATA_PORT);
        if (code < 128 && keyboard_map[code] != 0) {
            key_buffer[head] = keyboard_map[code];
            head = (head + 1) % BUFFER_SIZE;
            has_key_press = 1;
        }
    }
}

char getchar(void) {
    if (head == tail) return 0;
    char c = key_buffer[tail];
    tail = (tail + 1) % BUFFER_SIZE;
    if (head == tail) has_key_press = 0;
    return c;
}

int has_key(void) {
    return has_key_press;
}
