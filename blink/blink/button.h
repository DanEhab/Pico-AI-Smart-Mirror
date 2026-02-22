#ifndef BUTTON_H
#define BUTTON_H

#include "pico/stdlib.h"
#include <stdbool.h>

typedef struct {
    uint pin;
    bool active_low;   // true = press = LOW, false = press = HIGH
} button_t;

// Initialize button
void button_init(button_t *btn, uint pin, bool active_low);

// Read button state (true = pressed)
bool button_is_pressed(button_t *btn);

#endif
