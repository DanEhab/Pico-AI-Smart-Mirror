#include "motor_control.h"
#include "hardware/pwm.h" // Explicitly needed for PWM functions

// Function to initialize GPIO and PWM
void motor_init()
{
    // Set direction pins as output
    gpio_init(MOTOR_DIR_PIN1);
    gpio_set_dir(MOTOR_DIR_PIN1, GPIO_OUT);

    gpio_init(MOTOR_DIR_PIN2);
    gpio_set_dir(MOTOR_DIR_PIN2, GPIO_OUT);

    // Set PWM function for speed control
    gpio_set_function(MOTOR_PWM_PIN, GPIO_FUNC_PWM);

    // Get PWM slice number for GPIO 28
    uint slice_num = pwm_gpio_to_slice_num(MOTOR_PWM_PIN);

    // Set PWM configuration: wrap value and clock divider
    pwm_set_wrap(slice_num, 255);    // 8-bit resolution
    pwm_set_clkdiv(slice_num, 4.0f); // Set clock divider

    // Enable PWM output
    pwm_set_enabled(slice_num, true);
}

// Function to set motor speed and direction
void motor_control(uint16_t speed, bool forward)
{
    // Constrain speed between 0 and 255 (8-bit)
    if (speed > 255)
        speed = 255;

    // Set motor direction
    if (forward)
    {
        gpio_put(MOTOR_DIR_PIN1, 1);
        gpio_put(MOTOR_DIR_PIN2, 0);
    }
    else
    {
        gpio_put(MOTOR_DIR_PIN1, 0);
        gpio_put(MOTOR_DIR_PIN2, 1);
    }

    // Set PWM duty cycle to control speed
    pwm_set_gpio_level(MOTOR_PWM_PIN, speed);
}

// Stop the motor: disable PWM output and drive pins low
void motor_stop(void)
{
    // Determine slice and channel for the configured PWM pin
    uint slice_num = pwm_gpio_to_slice_num(MOTOR_PWM_PIN);
    uint chan = pwm_gpio_to_channel(MOTOR_PWM_PIN);

    // Set channel level to 0 and disable the slice
    pwm_set_chan_level(slice_num, chan, 0);
    pwm_set_enabled(slice_num, false);

    // Make sure the pin is driven low (safe state)
    gpio_set_function(MOTOR_PWM_PIN, GPIO_OUT);
    gpio_put(MOTOR_PWM_PIN, 0);

    // Also set direction pins low
    gpio_put(MOTOR_DIR_PIN1, 0);
    gpio_put(MOTOR_DIR_PIN2, 0);
}
