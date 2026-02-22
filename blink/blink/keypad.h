#ifndef KEYPAD_H
#define KEYPAD_H

#include "pico/stdlib.h"
#include <stdint.h>

// Modify these if you ever change wiring
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 3

typedef struct {
    uint8_t row_pins[KEYPAD_ROWS];
    uint8_t col_pins[KEYPAD_COLS];
    char key_map[KEYPAD_ROWS][KEYPAD_COLS];
} keypad_t;

// Initialize keypad with row pins, column pins, and key map
void keypad_init(keypad_t *keypad);

// Scan keypad once; return 0 if no key is pressed
char keypad_get_key(keypad_t *keypad);

#endif
