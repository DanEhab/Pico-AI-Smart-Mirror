#include "lcd_i2c.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

// LCD I2C address and hardware instance
static i2c_inst_t *i2c;
static uint8_t lcd_addr;

// LCD commands
#define LCD_CLEAR_DISPLAY 0x01
#define LCD_RETURN_HOME 0x02
#define LCD_ENTRY_MODE_SET 0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_CURSOR_SHIFT 0x10
#define LCD_FUNCTION_SET 0x20
#define LCD_SET_CGRAM_ADDR 0x40
#define LCD_SET_DDRAM_ADDR 0x80

// Flags for display entry mode
#define LCD_ENTRY_LEFT 0x02
#define LCD_ENTRY_SHIFT_DEC 0x00

// Flags for display on/off control
#define LCD_DISPLAY_ON 0x04
#define LCD_CURSOR_OFF 0x00
#define LCD_BLINK_OFF 0x00

// Flags for function set
#define LCD_4BIT_MODE 0x00
#define LCD_2_LINE 0x08
#define LCD_5x8_DOTS 0x00

// Backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NO_BACKLIGHT 0x00

// Enable bit
#define ENABLE 0x04

// Read/Write bit
#define RW 0x02

// Register select bit
#define RS 0x01

static uint8_t backlight_state = LCD_BACKLIGHT;

// Low-level I2C write function
static void i2c_write_byte(uint8_t val)
{
    i2c_write_blocking(i2c, lcd_addr, &val, 1, false);
}

// Pulse the enable bit to latch data
static void lcd_pulse_enable(uint8_t data)
{
    i2c_write_byte(data | ENABLE);
    sleep_us(1);
    i2c_write_byte(data & ~ENABLE);
    sleep_us(50);
}

// Write 4 bits to the LCD
static void lcd_write_nibble(uint8_t nibble, uint8_t mode)
{
    uint8_t data = (nibble & 0xF0) | mode | backlight_state;
    i2c_write_byte(data);
    lcd_pulse_enable(data);
}

// --- PUBLIC FUNCTIONS (Matching lcd_i2c.h) ---

// Write a byte to the LCD in 4-bit mode (Renamed from lcd_write_byte_mode)
void lcd_send_byte(uint8_t val, int mode)
{
    lcd_write_nibble(val & 0xF0, mode);        // Send high nibble
    lcd_write_nibble((val << 4) & 0xF0, mode); // Send low nibble
}

// Send a command to the LCD (Renamed from lcd_send_command)
void lcd_send_cmd(uint8_t cmd)
{
    lcd_send_byte(cmd, 0);
    sleep_us(2000); // 2ms wait for commands like Clear
}

// Send data (character) to the LCD
static void lcd_send_data(uint8_t data)
{
    lcd_send_byte(data, RS);
    sleep_us(50);
}

void lcd_init(void *i2c_port, uint sda_pin, uint scl_pin, uint8_t addr)
{
    i2c = (i2c_inst_t *)i2c_port;
    lcd_addr = addr;

    // Initialize I2C at 100kHz
    i2c_init(i2c, 100 * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    // Wait for LCD to power up
    sleep_ms(50);

    // Initialize LCD in 4-bit mode
    // Start in 8-bit mode
    lcd_write_nibble(0x30, 0);
    sleep_ms(5);
    lcd_write_nibble(0x30, 0);
    sleep_us(150);
    lcd_write_nibble(0x30, 0);
    sleep_us(150);

    // Switch to 4-bit mode
    lcd_write_nibble(0x20, 0);
    sleep_us(150);

    // Configure display: 4-bit mode, 2 lines, 5x8 font
    lcd_send_cmd(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2_LINE | LCD_5x8_DOTS);

    // Display on, cursor off, blink off
    lcd_send_cmd(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);

    // Clear display
    lcd_clear();

    // Entry mode: left to right, no shift
    lcd_send_cmd(LCD_ENTRY_MODE_SET | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DEC);

    sleep_ms(2);
}

void lcd_clear(void)
{
    lcd_send_cmd(LCD_CLEAR_DISPLAY);
    sleep_ms(2);
}

void lcd_set_cursor(int row, int col)
{
    // Row offsets for a 16x2 display
    uint8_t row_offsets[] = {0x00, 0x40};

    if (row > 1)
    {
        row = 1;
    }

    lcd_send_cmd(LCD_SET_DDRAM_ADDR | (col + row_offsets[row]));
}

void lcd_send_string(char *str)
{
    while (*str)
    {
        lcd_send_data(*str++);
    }
}

void lcd_send_char(char c)
{
    lcd_send_data(c);
}

void lcd_backlight_on(void)
{
    backlight_state = LCD_BACKLIGHT;
    i2c_write_byte(backlight_state);
}

void lcd_backlight_off(void)
{
    backlight_state = LCD_NO_BACKLIGHT;
    i2c_write_byte(backlight_state);
}