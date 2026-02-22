#ifndef LCD_I2C_H
#define LCD_I2C_H

#include "pico/stdlib.h"

/**
 * @brief Initializes the I2C LCD display.
 * 
 * This function configures the I2C hardware and initializes the LCD
 * in 4-bit mode with 2 lines and 5x8 font. It also turns on the display
 * and backlight.
 * 
 * @param i2c_port The I2C port to use (e.g., i2c0 or i2c1).
 * @param sda_pin The GPIO pin for SDA (data line).
 * @param scl_pin The GPIO pin for SCL (clock line).
 * @param addr The I2C address of the LCD (typically 0x27 or 0x3F).
 */
void lcd_init(void* i2c_port, uint sda_pin, uint scl_pin, uint8_t addr);

/**
 * @brief Clears the LCD display and returns cursor to home position.
 */
void lcd_clear(void);

/**
 * @brief Sets the cursor position on the LCD.
 * 
 * @param row The row number (0 or 1 for a 2-line display).
 * @param col The column number (0-15 for a 16-character display).
 */
void lcd_set_cursor(int row, int col);

/**
 * @brief Sends a string to the LCD at the current cursor position.
 * 
 * @param str Null-terminated string to display.
 */
void lcd_send_string(char *str);

/**
 * @brief Sends a single character to the LCD at the current cursor position.
 * 
 * @param c Character to display.
 */
void lcd_send_char(char c);

/**
 * @brief Turns the LCD backlight on.
 */
void lcd_backlight_on(void);

/**
 * @brief Turns the LCD backlight off.
 */
void lcd_backlight_off(void);

// --- NEWLY EXPOSED FUNCTIONS (Added for Custom Characters) ---
void lcd_send_cmd(uint8_t cmd);
void lcd_send_byte(uint8_t val, int mode);

#endif // LCD_I2C_H
