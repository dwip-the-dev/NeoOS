#include "keyboard.h"
#include "io.h"

// Keyboard ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Status register bits
#define KEYBOARD_STATUS_OUTPUT_BUFFER 0x01
#define KEYBOARD_STATUS_INPUT_BUFFER 0x02

// Commands to keyboard controller
#define KEYBOARD_CMD_ENABLE 0xF4
#define KEYBOARD_CMD_DISABLE 0xF5

// PIC master mask (unmask IRQ1 for keyboard)
#define PIC1_DATA 0x21

// Key buffer (circular queue)
#define KEY_BUFFER_SIZE 256
static uint16_t key_buffer[KEY_BUFFER_SIZE];  // Changed to uint16_t for extended scancodes
static int buffer_head = 0;
static int buffer_tail = 0;
static int buffer_count = 0;

// Shift state
static int shift_pressed = 0;
static int extended_prefix = 0;  // For E0 extended scancodes

// Special key codes (as defined in keyboard.h)
#define KEY_UP    -1
#define KEY_DOWN  -2
#define KEY_LEFT  -3
#define KEY_RIGHT -4
#define KEY_DELETE -5

// IRQ1 handler stub (called from IDT IRQ1)
void keyboard_handler(void);

// PS/2 scancode set 1 (US QWERTY layout)
char scancode_to_char(uint8_t scancode, int shift) {
    if (scancode == 0x00) return 0;  // Error or invalid

    // Handle release scancodes (0x80 bit set) - ignore for now
    if (scancode & 0x80) return 0;

    char c = 0;
    switch (scancode) {
        // Numbers and symbols
        case 0x02: c = shift ? '!' : '1'; break;
        case 0x03: c = shift ? '@' : '2'; break;
        case 0x04: c = shift ? '#' : '3'; break;
        case 0x05: c = shift ? '$' : '4'; break;
        case 0x06: c = shift ? '%' : '5'; break;
        case 0x07: c = shift ? '^' : '6'; break;
        case 0x08: c = shift ? '&' : '7'; break;
        case 0x09: c = shift ? '*' : '8'; break;
        case 0x0A: c = shift ? '(' : '9'; break;
        case 0x0B: c = shift ? ')' : '0'; break;
        case 0x0C: c = shift ? '_' : '-'; break;
        case 0x0D: c = shift ? '+' : '='; break;

        // Letters (A-Z)
        case 0x10: c = shift ? 'Q' : 'q'; break;
        case 0x11: c = shift ? 'W' : 'w'; break;
        case 0x12: c = shift ? 'E' : 'e'; break;
        case 0x13: c = shift ? 'R' : 'r'; break;
        case 0x14: c = shift ? 'T' : 't'; break;
        case 0x15: c = shift ? 'Y' : 'y'; break;
        case 0x16: c = shift ? 'U' : 'u'; break;
        case 0x17: c = shift ? 'I' : 'i'; break;
        case 0x18: c = shift ? 'O' : 'o'; break;
        case 0x19: c = shift ? 'P' : 'p'; break;
        case 0x1E: c = shift ? 'A' : 'a'; break;
        case 0x1F: c = shift ? 'S' : 's'; break;
        case 0x20: c = shift ? 'D' : 'd'; break;
        case 0x21: c = shift ? 'F' : 'f'; break;
        case 0x22: c = shift ? 'G' : 'g'; break;
        case 0x23: c = shift ? 'H' : 'h'; break;
        case 0x24: c = shift ? 'J' : 'j'; break;
        case 0x25: c = shift ? 'K' : 'k'; break;
        case 0x26: c = shift ? 'L' : 'l'; break;
        case 0x2C: c = shift ? 'Z' : 'z'; break;
        case 0x2D: c = shift ? 'X' : 'x'; break;
        case 0x2E: c = shift ? 'C' : 'c'; break;
        case 0x2F: c = shift ? 'V' : 'v'; break;
        case 0x30: c = shift ? 'B' : 'b'; break;
        case 0x31: c = shift ? 'N' : 'n'; break;
        case 0x32: c = shift ? 'M' : 'm'; break;

        // Punctuation and other
        case 0x1A: c = shift ? '{' : '['; break;
        case 0x1B: c = shift ? '}' : ']'; break;
        case 0x27: c = shift ? ':' : ';'; break;
        case 0x28: c = shift ? '"' : '\''; break;
        case 0x29: c = shift ? '~' : '`'; break;
        case 0x2B: c = shift ? '|' : '\\'; break;
        case 0x33: c = shift ? '<' : ','; break;
        case 0x34: c = shift ? '>' : '.'; break;
        case 0x35: c = shift ? '?' : '/'; break;

        // Special non-printable
        case 0x01: c = '\b'; break;  // Backspace
        case 0x0E: c = '\b'; break;  // Backspace (alt)
        case 0x1C: c = '\n'; break;  // Enter
        case 0x39: c = ' '; break;   // Space

        default:
            c = 0;
            break;
    }
    return c;
}

// Shift key scancodes
static void handle_modifier(uint8_t scancode) {
    if (scancode & 0x80) {  // Key release
        if (scancode == 0xAA || scancode == 0xB6) {  // Left/Right Shift release
            shift_pressed = 0;
        }
    } else {  // Key press
        if (scancode == 0x2A || scancode == 0x36) {  // Left/Right Shift press
            shift_pressed = 1;
        }
    }
}

// Buffer push (circular)
static void buffer_push(uint16_t scancode) {  // Changed to uint16_t
    if (buffer_count < KEY_BUFFER_SIZE) {
        key_buffer[buffer_tail] = scancode;
        buffer_tail = (buffer_tail + 1) % KEY_BUFFER_SIZE;
        buffer_count++;
    }
}

// Buffer pop
static uint16_t buffer_pop(void) {  // Changed to uint16_t
    if (buffer_count == 0) return 0;
    uint16_t scancode = key_buffer[buffer_head];
    buffer_head = (buffer_head + 1) % KEY_BUFFER_SIZE;
    buffer_count--;
    return scancode;
}

void keyboard_init(void) {
    // Wait for keyboard controller to be ready
    while (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_INPUT_BUFFER);

    // Disable keyboard
    outb(KEYBOARD_STATUS_PORT, KEYBOARD_CMD_DISABLE);
    while (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_INPUT_BUFFER);

    // Flush output buffer
    inb(KEYBOARD_DATA_PORT);

    // Enable keyboard
    outb(KEYBOARD_STATUS_PORT, KEYBOARD_CMD_ENABLE);
    while (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_INPUT_BUFFER);

    // Unmask IRQ1 in PIC (keyboard is IRQ1)
    uint8_t pic_mask = inb(PIC1_DATA) & ~0x02;  // Clear bit 1
    outb(PIC1_DATA, pic_mask);

    // Ensure IRQ1 is set in IDT (handled in kernel.c)
    buffer_head = 0;
    buffer_tail = 0;
    buffer_count = 0;
    shift_pressed = 0;
    extended_prefix = 0;

    // Self-test or ack (optional)
    uint8_t ack = inb(KEYBOARD_DATA_PORT);
    (void)ack;  // Ignore for now
}

// Poll for keyboard input (non-blocking, for shell loop)
void poll_keyboard(void) {
    if (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_BUFFER) {
        uint8_t scancode = inb(KEYBOARD_DATA_PORT);

        // Handle extended prefix
        if (scancode == 0xE0) {
            extended_prefix = 1;
            return;
        }

        // Handle scancode (press/release)
        if (scancode & 0x80) {
            // Key release
            uint8_t key_press = scancode & 0x7F;
            handle_modifier(key_press);
            if (extended_prefix) {
                extended_prefix = 0;  // Ignore extended releases
            }
        } else {
            // Key press
            handle_modifier(scancode);
            if (extended_prefix) {
                // Extended press: arrows, delete
                switch (scancode) {
                    case 0x48: buffer_push(0xE048); break;  // Up
                    case 0x50: buffer_push(0xE050); break;  // Down
                    case 0x4B: buffer_push(0xE04B); break;  // Left
                    case 0x4D: buffer_push(0xE04D); break;  // Right
                    case 0x53: buffer_push(0xE053); break;  // Delete
                    default: break;
                }
                extended_prefix = 0;
            } else {
                // Regular key press
                buffer_push(scancode);
            }
        }
    }
}

int has_key(void) {
    return buffer_count > 0;
}

int getkey(void) {
    if (!has_key()) return 0;

    uint16_t scancode = buffer_pop();

    // Handle extended/special keys
    if (scancode == 0xE048) return KEY_UP;
    if (scancode == 0xE050) return KEY_DOWN;
    if (scancode == 0xE04B) return KEY_LEFT;
    if (scancode == 0xE04D) return KEY_RIGHT;
    if (scancode == 0xE053) return KEY_DELETE;

    // Convert to char (scancode is <= 255 here)
    char c = scancode_to_char((uint8_t)scancode, shift_pressed);
    return (int)(uint8_t)c;  // Cast to ensure positive ASCII
}

// IRQ1 handler (called from assembly stub)
void keyboard_handler(void) {
    // Since poll_keyboard handles buffering, just clear the interrupt
    // Read scancode to ack IRQ
    if (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_BUFFER) {
        (void)inb(KEYBOARD_DATA_PORT);  // Read to clear
    }
    // Send EOI to PIC
    outb(0x20, 0x20);
}
