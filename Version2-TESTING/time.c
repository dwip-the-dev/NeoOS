#include "time.h"

static uint8_t hours = 0;
static uint8_t minutes = 0;
static uint8_t seconds = 0;

void set_time(uint8_t hh, uint8_t mm, uint8_t ss) {
    if (hh < 24 && mm < 60 && ss < 60) {
        hours = hh;
        minutes = mm;
        seconds = ss;
    }
}

void time_to_string(char* buffer) {
    buffer[0] = '0' + hours / 10;
    buffer[1] = '0' + hours % 10;
    buffer[2] = ':';
    buffer[3] = '0' + minutes / 10;
    buffer[4] = '0' + minutes % 10;
    buffer[5] = ':';
    buffer[6] = '0' + seconds / 10;
    buffer[7] = '0' + seconds % 10;
    buffer[8] = '\0';
}

void increment_time(void) {
    seconds++;
    if (seconds >= 60) {
        seconds = 0;
        minutes++;
        if (minutes >= 60) {
            minutes = 0;
            hours++;
            if (hours >= 24) {
                hours = 0;
            }
        }
    }
}

// Helper function to parse 1-2 digits from string
static int parse_digits(const char* str, int start, int* val) {
    if (str[start] < '0' || str[start] > '9') return 0;
    *val = str[start] - '0';
    if (str[start + 1] >= '0' && str[start + 1] <= '9') {
        *val = *val * 10 + (str[start + 1] - '0');
        return (str[start + 2] == ':') ? 3 : 0;  // Return offset if valid separator
    }
    return (str[start + 1] == ':') ? 2 : 0;  // Single digit, check separator
}

// Manual parser for HH:MM:SS (supports 1-2 digits per field)
int parse_time_str(const char* str, uint8_t* hh, uint8_t* mm, uint8_t* ss) {
    if (!str || !hh || !mm || !ss) return 0;

    int idx = 0;
    int h = 0, m = 0, s = 0;

    // Parse hours
    int offset = parse_digits(str, 0, &h);
    if (!offset) return 0;
    idx += offset;

    // Parse minutes
    offset = parse_digits(str, idx, &m);
    if (!offset) return 0;
    idx += offset;

    // Parse seconds
    offset = parse_digits(str, idx, &s);
    if (!offset || str[idx + offset] != '\0') return 0;  // Ensure end of string

    // Validate ranges
    if (h > 23 || m > 59 || s > 59) return 0;

    *hh = (uint8_t)h;
    *mm = (uint8_t)m;
    *ss = (uint8_t)s;
    return 1;
}
