
#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "pico/stdlib.h"

// GPIO Pins for Motor Control
#define MOTOR_PWM_PIN 28  // GPIO 28 for PWM (speed control) - Pin 34
#define MOTOR_DIR_PIN1 21 // GPIO 21 for motor direction - Pin 27
#define MOTOR_DIR_PIN2 22 // GPIO 22 for motor direction - Pin 29

// Function Declarations
void motor_init(void);
void motor_control(uint16_t speed, bool forward);
void motor_stop(void);

#endif // MOTOR_CONTROL_H