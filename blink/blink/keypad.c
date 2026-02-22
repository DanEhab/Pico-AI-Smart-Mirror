#include "keypad.h"

void keypad_init(keypad_t *keypad)
{
    // Initialize row pins as outputs
    for (int r = 0; r < KEYPAD_ROWS; r++)
    {
        gpio_init(keypad->row_pins[r]);
        gpio_set_dir(keypad->row_pins[r], GPIO_OUT);
        gpio_put(keypad->row_pins[r], 0); // Start LOW
    }

    // Initialize column pins as inputs with pull-down
    for (int c = 0; c < KEYPAD_COLS; c++)
    {
        gpio_init(keypad->col_pins[c]);
        gpio_set_dir(keypad->col_pins[c], GPIO_IN);
        gpio_pull_down(keypad->col_pins[c]);
    }
}

char keypad_get_key(keypad_t *keypad)
{
    for (int r = 0; r < KEYPAD_ROWS; r++)
    {

        // Drive only this row HIGH
        gpio_put(keypad->row_pins[r], 1);

        // Check columns
        for (int c = 0; c < KEYPAD_COLS; c++)
        {
            if (gpio_get(keypad->col_pins[c]))
            {

                // Simple debounce
                sleep_ms(20);
                while (gpio_get(keypad->col_pins[c]))
                    ; // wait release

                gpio_put(keypad->row_pins[r], 0); // turn row off
                return keypad->key_map[r][c];
            }
        }

        gpio_put(keypad->row_pins[r], 0); // turn row off before next row
    }

    return 0; // no key pressed
}
