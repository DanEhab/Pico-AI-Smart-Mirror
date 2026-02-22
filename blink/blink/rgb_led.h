#ifndef RGB_LED_H
#define RGB_LED_H

#include "pico/stdlib.h"
#include <stdbool.h>

/**
 * @brief Initializes the RGB LED on the specified GPIO pins.
 *
 * This function configures hardware PWM for the red, green, and blue channels
 * of an RGB LED.
 *
 * @param r_pin GPIO pin connected to the Red LED anode.
 * @param g_pin GPIO pin connected to the Green LED anode.
 * @param b_pin GPIO pin connected to the Blue LED anode.
 * @param common_anode Set to true for common anode LED, false for common cathode.
 */
void rgb_led_init(uint r_pin, uint g_pin, uint b_pin, bool common_anode);

/**
 * @brief Sets the color of the RGB LED.
 *
 * For a common cathode LED, 0 = LED off, 255 = full brightness.
 *
 * @param r Red brightness (0-255).
 * @param g Green brightness (0-255).
 * @param b Blue brightness (0-255).
 */
void rgb_led_set_color(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Turns off the RGB LED (all channels to 0).
 */
void rgb_led_off(void);

#endif // RGB_LED_H
