#ifndef TIME_H
#define TIME_H

#include <stdint.h>

// Set the current time manually
void set_time(uint8_t hh, uint8_t mm, uint8_t ss);

// Convert current time to string (HH:MM:SS), requires buffer of at least 9 bytes
void time_to_string(char* buffer);

// Increment the time by one second (used by PIT handler)
void increment_time(void);

// Parse time string in format HH:MM:SS, returns 1 if valid, 0 if invalid
int parse_time_str(const char* str, uint8_t* hh, uint8_t* mm, uint8_t* ss);

#endif // TIME_H
