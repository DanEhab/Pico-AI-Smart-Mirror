#include "rgb_led.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

// Store GPIO pin assignments
static uint red_pin;
static uint green_pin;
static uint blue_pin;

// Store PWM slice numbers for each pin
static uint red_slice;
static uint green_slice;
static uint blue_slice;

// Common anode flag (inverts output)
static bool is_common_anode = false;

// PWM wrap value (determines PWM resolution)
// Using 255 for direct mapping of 0-255 brightness values
#define PWM_WRAP_VALUE 255

void rgb_led_init(uint r_pin, uint g_pin, uint b_pin, bool common_anode)
{
    // Store pin assignments and LED type
    red_pin = r_pin;
    green_pin = g_pin;
    blue_pin = b_pin;
    is_common_anode = common_anode;

    // Configure Red channel
    gpio_set_function(red_pin, GPIO_FUNC_PWM);
    red_slice = pwm_gpio_to_slice_num(red_pin);
    pwm_set_wrap(red_slice, PWM_WRAP_VALUE);
    pwm_set_enabled(red_slice, true);

    // Configure Green channel
    gpio_set_function(green_pin, GPIO_FUNC_PWM);
    green_slice = pwm_gpio_to_slice_num(green_pin);
    pwm_set_wrap(green_slice, PWM_WRAP_VALUE);
    pwm_set_enabled(green_slice, true);

    // Configure Blue channel
    gpio_set_function(blue_pin, GPIO_FUNC_PWM);
    blue_slice = pwm_gpio_to_slice_num(blue_pin);
    pwm_set_wrap(blue_slice, PWM_WRAP_VALUE);
    pwm_set_enabled(blue_slice, true);

    // Initialize to off
    rgb_led_off();
}

void rgb_led_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    // For common cathode: 0 = off, 255 = full brightness
    // For common anode: invert (0 = full brightness, 255 = off)
    if (is_common_anode)
    {
        r = 255 - r;
        g = 255 - g;
        b = 255 - b;
    }

    // Set all channels immediately without delay
    pwm_set_gpio_level(red_pin, r);
    pwm_set_gpio_level(green_pin, g);
    pwm_set_gpio_level(blue_pin, b);
}

void rgb_led_off(void)
{
    rgb_led_set_color(0, 0, 0);
}
