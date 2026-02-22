#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/watchdog.h" // Needed for the reboot command

// Include your existing drivers
#include "button.h"
#include "keypad.h"
#include "rgb_led.h"
#include "lcd_i2c.h"
#include "motor_control.h"


// 1. Bitmap for a "Beamed Eighth Note" ♫
uint8_t music_note[8] = {
    0b00001,
    0b00011,
    0b00101,
    0b01001,
    0b01001,
    0b01011,
    0b11011,
    0b11000
};

// --- BIG SURPRISE FACE PARTS ---

// Slot 2: Top Left (Head outline + Left Eye)
uint8_t big_face_TL[8] = {
    0b00111,
    0b01000,
    0b10000,
    0b10010, // Left Eye
    0b10101, // Left Eye Pupil
    0b10010, // Left Eye
    0b10000,
    0b10000
};

// Slot 3: Top Right (Head outline + Right Eye)
uint8_t big_face_TR[8] = {
    0b11100,
    0b00010,
    0b00001,
    0b01001, // Right Eye
    0b10101, // Right Eye Pupil
    0b01001, // Right Eye
    0b00001,
    0b00001
};

// Slot 4: Bottom Left (Chin + Left side of open mouth)
uint8_t big_face_BL[8] = {
    0b10000,
    0b10011, // Mouth Top
    0b10100, // Mouth Side
    0b10100, // Mouth Side
    0b10011, // Mouth Bottom
    0b01000,
    0b00111,
    0b00000
};

// Slot 5: Bottom Right (Chin + Right side of open mouth)
uint8_t big_face_BR[8] = {
    0b00001,
    0b11001, // Mouth Top
    0b00101, // Mouth Side
    0b00101, // Mouth Side
    0b11001, // Mouth Bottom
    0b00010,
    0b11100,
    0b00000
};

// 2. Helper to upload custom characters
void lcd_create_char(uint8_t location, uint8_t *charmap) {
    location &= 0x7; // We only have 8 slots (0-7)
    lcd_send_cmd(0x40 | (location << 3)); // Set CGRAM address
    for (int i = 0; i < 8; i++) {
        lcd_send_byte(charmap[i], 1); // Send data
    }
}

// Function to rotate mirror ~180 degrees
void rotate_mirror_180() {
    // Start motor at slow speed (80/255 = ~31%) for controlled rotation
    motor_control(170, true);  // true = forward, false = reverse
    
    // Run for ~250ms to achieve approximately 180 degree rotation
    // Adjust this value during testing if needed (increase = more rotation)
    sleep_ms(400);
    
    // Stop motor completely
    motor_stop();
}

// ---------------- PINS ----------------
#define BUTTON_PIN 16

#define LED_R 18
#define LED_G 19
#define LED_B 20

#define I2C_SDA 26
#define I2C_SCL 27
#define LCD_ADDR 0x27

// ------------- PASSWORD ---------------
#define PASSWORD "162"
#define PASS_LEN 3

// ------------- HELPER FUNCTIONS -------

// Helper to send text-to-speech command over serial
void speak(const char* text) {
    printf("TTS:%s\n", text);
    fflush(stdout);
    sleep_ms(100); // Small delay to ensure message is sent before next LCD update
}

// Helper to check for a "Long Press" reset command
void check_reset(button_t *btn) {
    if (button_is_pressed(btn)) {
        uint32_t start_time = to_ms_since_boot(get_absolute_time());
        while (button_is_pressed(btn)) {
            // If held for > 2000ms (2 seconds), REBOOT
            if (to_ms_since_boot(get_absolute_time()) - start_time > 2000) {
                lcd_clear();
                lcd_set_cursor(0, 0);
                lcd_send_string("Rebooting...");
                speak("Rebooting system");
                rgb_led_set_color(0, 0, 255); // Blue indicates reboot
                sleep_ms(500);
                watchdog_reboot(0, 0, 0); // Trigger a full system reset
            }
            sleep_ms(10);
        }
    }
}

// Wrapper for sleep_ms that also checks for reset
void sleep_and_check(uint32_t ms, button_t *btn) {
    uint32_t end_time = to_ms_since_boot(get_absolute_time()) + ms;
    while (to_ms_since_boot(get_absolute_time()) < end_time) {
        check_reset(btn);
        sleep_ms(10);
    }
}


// Helper to trigger music on the laptop
void play_music() {
    printf("MUSIC:PLAY\n"); // Send command to Python
    fflush(stdout);
    sleep_ms(100);
}

// Helper to trigger the surprise sound on laptop
void play_surprise_sound() {
    printf("SURPRISE:PLAY\n"); // Unique command different from "MUSIC:"
    fflush(stdout);
    sleep_ms(100);
}

int main()
{
    // Initialize USB Serial for Python communication
    stdio_init_all();

    // Wait for power to stabilize
    sleep_ms(500);
    
    // -------- 1. INITIALIZE HARDWARE --------
    button_t button;
    button_init(&button, BUTTON_PIN, false); 

    rgb_led_init(LED_R, LED_G, LED_B, false);

    // Blink LED to indicate power-on (visual confirmation)
    rgb_led_set_color(255, 0, 0);
    sleep_ms(200);
    rgb_led_off();
    sleep_ms(200);
    
    // Initialize I2C with hardware reset sequence
    lcd_init(i2c1, I2C_SDA, I2C_SCL, LCD_ADDR);

    // --- UPLOAD BIG FACE PARTS ---
    lcd_create_char(1, music_note);  // Slot 2: Top Left
    lcd_create_char(2, big_face_TL);  // Slot 2: Top Left
    lcd_create_char(3, big_face_TR);  // Slot 3: Top Right
    lcd_create_char(4, big_face_BL);  // Slot 4: Bot Left
    lcd_create_char(5, big_face_BR);  // Slot 5: Bot Right

    // Send I2C reset sequence to LCD
    sleep_ms(100);
    lcd_backlight_on();
    sleep_ms(50);
    lcd_clear();
    sleep_ms(50);

    keypad_t keypad = {
        .row_pins = {6, 7, 8, 9},
        .col_pins = {10, 11, 12},
        .key_map = {
            {'1', '2', '3'},
            {'4', '5', '6'},
            {'7', '8', '9'},
            {'*', '0', '#'}}};
    keypad_init(&keypad);
    
    // Initialize motor control
    motor_init();

    // -------- MOTOR TEST (First 5 seconds) --------
    /*lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_send_string("Testing Motor");
    speak("Testing Motor 180 degree flip");
    lcd_set_cursor(1, 0);
    lcd_send_string("180 deg flip");
    rgb_led_set_color(0, 255, 0); // Green during test
    
    // Rotate ~180 degrees (half turn) - adjust if needed
    motor_control(170, true);   // slowest = 170, fastest = 255
    sleep_ms(400);             // 250ms for ~180 degree rotation
    motor_stop();              // Ensure PWM fully disabled and pins driven low
    */
    /*lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_send_string("Motor Stopped");
    speak("Motor Stopped. Test complete");
    lcd_set_cursor(1, 0);
    lcd_send_string("Test complete");
    sleep_ms(2000);*/
    
    // -------- 2. WELCOME SEQUENCE --------
    restart_system: // Label for system restart after Phase 3 completion
    
    // Part A: "Welcome to Station 6" with Blinking LED
    sleep_ms(3200);
    speak("");
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_send_string("Welcome to");
    lcd_set_cursor(1, 0);
    speak("Welcome to Station 6");
    lcd_send_string("Station 6");

    // RGB Blinks colors for 5 seconds
    for (int i = 0; i < 5; i++) {
        rgb_led_set_color(255, 0, 0); 
        sleep_and_check(333, &button); 
        rgb_led_set_color(0, 255, 0); 
        sleep_and_check(333, &button);
        rgb_led_set_color(0, 0, 255); 
        sleep_and_check(333, &button);
    }
    
    // Part B: Safety Instructions
    rgb_led_set_color(0, 0, 255); // Set to steady Blue for info messages

    // "Pls focus on upcoming instructions" - Show for 5 seconds
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_send_string("Pls focus on");
    speak("Please focus on upcoming instructions");
    lcd_set_cursor(1, 0);
    lcd_send_string("upcoming instr.");
    sleep_and_check(5000, &button); 

    // "Reset = hold button for 2 seconds" - Show for 5 seconds
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_send_string("Reset = hold btn");
    speak("Reset equals hold button for 2 seconds");
    lcd_set_cursor(1, 0);
    lcd_send_string("for 2 seconds");
    sleep_and_check(5000, &button); 

    // "DONT Touch any of the wires" - Show for 5 seconds
    // Flash RED for warning
    rgb_led_set_color(255, 0, 0);
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_send_string("DONT Touch any");
    speak("DON'T Touch any of the wires!");
    lcd_set_cursor(1, 0);
    lcd_send_string("of the wires!");
    sleep_and_check(5000, &button); 

    // "Trust me! Its for ur own safety :)" - Show for 5 seconds
    // Back to calm Blue
    rgb_led_set_color(0, 0, 255);
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_send_string("Trust me! Its");
    speak("Trust me! It's for your own safety");
    lcd_set_cursor(1, 0);
    lcd_send_string("for ur safety :)");
    sleep_and_check(5000, &button); 

    rgb_led_off();

    // -------- 3. MAIN LOOP --------
    while (1)
    {
        // Always check for reset at start of loop
        check_reset(&button);

        // State: "Input Mode"
        rgb_led_set_color(255, 0, 0); // Steady Red waiting for input

        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_send_string("Pls enter your");
        speak("Please enter your 3 digits");
        lcd_set_cursor(1, 0);
        lcd_send_string("3 digits...");

        int idx = 0;
        bool code_failed = false;

        // Loop to read exactly 3 digits
        while (idx < PASS_LEN)
        {
            // Check for reset constantly while waiting for key
            check_reset(&button);

            char key = keypad_get_key(&keypad);
            char key_str[2] = {key, '\0'}; // Convert char '1' to string "1"
            speak(key_str);
                     
            if (key)
            {
                if (idx == 0) {
                    lcd_set_cursor(1, 0);
                    lcd_send_string("                "); 
                    lcd_set_cursor(1, 0); 
                }

                lcd_send_char(key);

                if (key == PASSWORD[idx])
                {
                    rgb_led_set_color(0, 255, 0); 
                    sleep_and_check(1000, &button);
                    rgb_led_set_color(255, 0, 0); 
                }
                else
                {
                    code_failed = true;
                    for(int i=0; i<5; i++) {
                        rgb_led_off();
                        sleep_and_check(100, &button);
                        rgb_led_set_color(255, 0, 0);
                        sleep_and_check(100, &button);
                    }
                    rgb_led_set_color(255, 0, 0);
                }
                idx++; 
            }
            sleep_ms(10);
        }

        // -------- 4. CHECK CODE & TRIGGER AI --------
        lcd_clear();
        
        if (!code_failed)
        {
            // --- SUCCESS: Code Correct ---
            lcd_set_cursor(0, 0);
            lcd_send_string("Code Accepted!");
            speak("Code Accepted! Press Button!");
            lcd_set_cursor(1, 0);
            lcd_send_string("Press Button!");

            // Flash Green briefly to confirm code
            for(int i=0; i<3; i++) {
                rgb_led_set_color(0, 255, 0);
                sleep_and_check(250, &button);
                rgb_led_off();
                sleep_and_check(250, &button);
            }

            // --- WAIT FOR BUTTON PRESS ---
            // Blink Green slowly - waiting for button
            bool button_pressed = false;
            uint32_t last_blink = 0;
            bool led_state = false;
            
            while (!button_pressed) {
                // Blink LED to show we're waiting
                uint32_t now = to_ms_since_boot(get_absolute_time());
                if (now - last_blink > 500) {
                    led_state = !led_state;
                    if (led_state) {
                        rgb_led_set_color(0, 255, 0);
                    } else {
                        rgb_led_off();
                    }
                    last_blink = now;
                }
                
                // Check button with simpler logic
                if (button_is_pressed(&button)) {
                    // Visual feedback - button detected
                    rgb_led_set_color(255, 255, 0); // Yellow
                    sleep_ms(100);
                    
                    // Wait for release
                    while (button_is_pressed(&button)) {
                        sleep_ms(10);
                    }
                    
                    button_pressed = true;
                }
                
                sleep_ms(10);
            }

            // --- BUTTON PRESSED - GAME INSTRUCTIONS ---
            
            // Message 1: "Get Excited! Game will start now"
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_send_string("Get Excited!");
            speak("Get Excited! Game starts now");
            lcd_set_cursor(1, 0);
            lcd_send_string("Game starts now");
            rgb_led_set_color(255, 255, 0); // Yellow - excitement
            sleep_and_check(3000, &button);
            
            // Message 2: "You will need to do a face expression"
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_send_string("You will need");
            speak("You will need to do a face expression");
            lcd_set_cursor(1, 0);
            lcd_send_string("face expression");
            rgb_led_set_color(0, 255, 255); // Cyan - info
            sleep_and_check(3000, &button);
            
            // Message 3: "You need to maintain it for 10 sec."
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_send_string("Maintain it for");
            speak("Maintain it for 5 seconds!");
            lcd_set_cursor(1, 0);
            lcd_send_string("5 seconds!");
            rgb_led_set_color(255, 0, 255); // Magenta - info
            sleep_and_check(3000, &button);
            
            // Message 4: "You only have 20 second!!"
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_send_string("You only have");
            speak("You only have 20 seconds for each phase!!");
            lcd_set_cursor(1, 0);
            lcd_send_string("20 seconds!!");
            rgb_led_set_color(255, 0, 0); // Red - urgency
            sleep_and_check(3000, &button);
            
            // Message 5: "Press Button after reading hint"
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_send_string("Press Button");
            speak("Press Button after reading hint");
            lcd_set_cursor(1, 0);
            lcd_send_string("after hint");
            sleep_and_check(3000, &button);
            
            /*
            // Blink Orange - waiting for button
            button_pressed = false;
            last_blink = 0;
            led_state = false;
            
            while (!button_pressed) {
                // Blink LED to show we're waiting
                uint32_t now = to_ms_since_boot(get_absolute_time());
                if (now - last_blink > 500) {
                    led_state = !led_state;
                    if (led_state) {
                        rgb_led_set_color(255, 165, 0); // Orange
                    } else {
                        rgb_led_off();
                    }
                    last_blink = now;
                }
                
                // Check button
                if (button_is_pressed(&button)) {
                    // Visual feedback
                    rgb_led_set_color(255, 255, 0); // Yellow
                    sleep_ms(100);
                    
                    // Wait for release
                    while (button_is_pressed(&button)) {
                        sleep_ms(10);
                    }
                    
                    button_pressed = true;
                }
                
                sleep_ms(10);
            }
            */

            // --- PUZZLE 1 HINT ---
            phase1_start: // Label for restart after failure
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_send_string("Ersem alb w ...??");
            lcd_set_cursor(1, 0);
            lcd_send_string("3al shababeek\1");
            rgb_led_set_color(128, 0, 128); // Purple - puzzle hint
            speak("Phase 1:");
            sleep_and_check(1000, &button);
            play_music();
            sleep_and_check(17000, &button);
            
            /*lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_send_string("...GRIN lives");
            lcd_set_cursor(1, 0);
            lcd_send_string("on!");*/
            sleep_and_check(2000, &button);
            
            // Wait for button press to start
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_send_string("Press Button to");
            speak("Press Button to start!");
            lcd_set_cursor(1, 0);
            lcd_send_string("start!");
            
            button_pressed = false;
            last_blink = 0;
            led_state = false;
            
            while (!button_pressed) {
                uint32_t now = to_ms_since_boot(get_absolute_time());
                if (now - last_blink > 500) {
                    led_state = !led_state;
                    if (led_state) {
                        rgb_led_set_color(0, 255, 0);
                    } else {
                        rgb_led_off();
                    }
                    last_blink = now;
                }
                
                if (button_is_pressed(&button)) {
                    rgb_led_set_color(255, 255, 0);
                    sleep_ms(100);
                    while (button_is_pressed(&button)) {
                        sleep_ms(10);
                    }
                    button_pressed = true;
                }
                
                sleep_ms(10);
            }
            
            // --- NOW START AI ---
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_send_string("Starting AI...");
            speak("Starting A I");
            
            // Send "START" signal to Python
            printf("START\n");
            fflush(stdout);
            
            rgb_led_set_color(0, 0, 255);
            sleep_ms(1000);
            
            // Show 20 second countdown with instruction
            lcd_clear();
            lcd_set_cursor(0, 0);
            //lcd_send_string("Smile for 10s!");
            //speak("Smile for 10 seconds!");
            
            // --- 20 SECOND COUNTDOWN WITH DIGIT MONITORING ---
            uint32_t countdown_start = to_ms_since_boot(get_absolute_time());
            uint32_t countdown_duration = 20000; // 20 seconds in milliseconds
            bool success = false;
            char last_digit = '-';
            
            while(1) {
                check_reset(&button);
                
                uint32_t elapsed = to_ms_since_boot(get_absolute_time()) - countdown_start;
                uint32_t remaining = (countdown_duration - elapsed) / 1000; // seconds remaining
                
                // Check if time is up
                if (elapsed >= countdown_duration) {
                    // Time's up - failed Phase 1
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_send_string("Hard Luck!");
                    speak("Hard Luck! Try Again!");
                    lcd_set_cursor(1, 0);
                    lcd_send_string("Try Again!");
                    
                    rgb_led_set_color(255, 0, 0); // Red - failed
                    sleep_and_check(3000, &button);
                    
                    // Return to Phase 1 hint and restart
                    goto phase1_start;
                }
                
                // Update countdown display
                lcd_set_cursor(1, 0);
                char countdown_str[17];
                snprintf(countdown_str, sizeof(countdown_str), "Time: %02lus", remaining);
                lcd_send_string(countdown_str);
                
                // Check for incoming data from Python
                int c = getchar_timeout_us(0);
                
                if (c != PICO_ERROR_TIMEOUT) {
                    // Check if it's a digit character (0-9)
                    if (c >= '0' && c <= '9') {
                        last_digit = (char)c;
                        
                        // Flash Green briefly to show digit received
                        rgb_led_set_color(0, 255, 0);
                        sleep_ms(100);
                        rgb_led_set_color(0, 0, 255);
                    }
                    // Check for "SUCCESS1" message from Python (Phase 1 - Smile only)
                    else if (c == 'S') {
                        // Try to read next few characters
                        char buffer[12];
                        buffer[0] = 'S';
                        int buf_idx = 1;
                        
                        // Read rest of message
                        for (int i = 0; i < 20 && buf_idx < 11; i++) {
                            int next_c = getchar_timeout_us(10000); // 10ms timeout per char
                            if (next_c != PICO_ERROR_TIMEOUT) {
                                buffer[buf_idx++] = (char)next_c;
                                if (next_c == '\n' || next_c == '\r') break;
                            }
                        }
                        buffer[buf_idx] = '\0';
                        
                        // Check if it says "SUCCESS1" (for Phase 1 - digit 1 only)
                        if (strstr(buffer, "SUCCESS1") != NULL) {
                            success = true;
                            break;
                        }
                    }
                }
                
                sleep_ms(50);
            }
            
            // --- CHECK PHASE 1 SUCCESS ---
            if (success) {
                lcd_clear();
                lcd_set_cursor(0, 0);
                lcd_send_string("Good Job 1!");
                speak("Good Job! Phase 1 Challenge Done!");
                lcd_set_cursor(1, 0);
                lcd_send_string("Challenge Done!");
                
                // Flash Green - success!
                for (int i = 0; i < 6; i++) {
                    rgb_led_set_color(0, 255, 0);
                    sleep_ms(250);
                    rgb_led_off();
                    sleep_ms(250);
                }
                
                sleep_and_check(2000, &button);
                
                // ============ PHASE 2: SURPRISE ============
                
                // --- PUZZLE 2 HINT ---
                lcd_clear();
                lcd_set_cursor(0, 0);
                lcd_set_cursor(0, 0);
                lcd_send_string("Surprise!   \2\3"); // \2 and \3 are the top eyes
                
                // ROW 1: Text + Bottom half of face
                lcd_set_cursor(1, 0);
                lcd_send_string("Show Me!!   \4\5"); // \4 and \5 are the open mouth
                speak("Phase 2:");
                sleep_and_check(1000, &button);
                play_surprise_sound();
                rgb_led_set_color(255, 255, 0); // Yellow
                sleep_and_check(7000, &button);
                
                // Wait for button press to start Phase 2
                lcd_clear();
                lcd_set_cursor(0, 0);
                lcd_send_string("Press Button to");
                speak("Press Button to start Phase 2!");
                lcd_set_cursor(1, 0);
                lcd_send_string("start Phase 2!");
                
                button_pressed = false;
                last_blink = 0;
                led_state = false;
                
                while (!button_pressed) {
                    uint32_t now = to_ms_since_boot(get_absolute_time());
                    if (now - last_blink > 500) {
                        led_state = !led_state;
                        if (led_state) {
                            rgb_led_set_color(0, 255, 0);
                        } else {
                            rgb_led_off();
                        }
                        last_blink = now;
                    }
                    
                    if (button_is_pressed(&button)) {
                        rgb_led_set_color(255, 255, 0);
                        sleep_ms(100);
                        while (button_is_pressed(&button)) {
                            sleep_ms(10);
                        }
                        button_pressed = true;
                    }
                    
                    sleep_ms(10);
                }
                
                // Start Phase 2 countdown
                lcd_clear();
                lcd_set_cursor(0, 0);
                //lcd_send_string("Surprise for10s");
                //speak("Show Surprise for 10 seconds!");
                
                rgb_led_set_color(0, 0, 255);
                sleep_ms(500);
                
                // --- PHASE 2: 20 SECOND COUNTDOWN ---
                countdown_start = to_ms_since_boot(get_absolute_time());
                success = false; // Reset for Phase 2
                
                while(1) {
                    check_reset(&button);
                    
                    uint32_t elapsed = to_ms_since_boot(get_absolute_time()) - countdown_start;
                    uint32_t remaining = (countdown_duration - elapsed) / 1000;
                    
                    // Check if time is up
                    if (elapsed >= countdown_duration) {
                        // Time's up - failed Phase 2
                        lcd_clear();
                        lcd_set_cursor(0, 0);
                        lcd_send_string("Hard Luck!");
                        speak("Hard Luck! Try Again!");
                        lcd_set_cursor(1, 0);
                        lcd_send_string("Try Again!");
                        
                        rgb_led_set_color(255, 0, 0);
                        sleep_and_check(3000, &button);
                        
                        // Return to Phase 1 hint and restart
                        goto phase1_start;
                    }
                    
                    // Update countdown display
                    lcd_set_cursor(1, 0);
                    char countdown_str[17];
                    snprintf(countdown_str, sizeof(countdown_str), "Time: %02lus", remaining);
                    lcd_send_string(countdown_str);
                    
                    // Check for incoming data from Python
                    int c = getchar_timeout_us(0);
                    
                    if (c != PICO_ERROR_TIMEOUT) {
                        // Check if it's a digit character (0-9)
                        if (c >= '0' && c <= '9') {
                            last_digit = (char)c;
                            
                            // Flash Green briefly
                            rgb_led_set_color(0, 255, 0);
                            sleep_ms(100);
                            rgb_led_set_color(0, 0, 255);
                        }
                        // Check for "SUCCESS6" message (Phase 2 - Surprise only)
                        else if (c == 'S') {
                            char buffer[12];
                            buffer[0] = 'S';
                            int buf_idx = 1;
                            
                            for (int i = 0; i < 20 && buf_idx < 11; i++) {
                                int next_c = getchar_timeout_us(10000);
                                if (next_c != PICO_ERROR_TIMEOUT) {
                                    buffer[buf_idx++] = (char)next_c;
                                    if (next_c == '\n' || next_c == '\r') break;
                                }
                            }
                            buffer[buf_idx] = '\0';
                            
                            // Check if it says "SUCCESS6" (for Phase 2 - digit 6 only)
                            if (strstr(buffer, "SUCCESS6") != NULL) {
                                success = true;
                                break;
                            }
                        }
                    }
                    
                    sleep_ms(50);
                }
                
                // --- CHECK PHASE 2 SUCCESS ---
                if (success) {
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_send_string("Good Job 6!");
                    speak("Good Job! Phase 2 Challenge Done!");
                    lcd_set_cursor(1, 0);
                    lcd_send_string("Challenge Done!");
                    
                    // Flash Green - success!
                    for (int i = 0; i < 6; i++) {
                        rgb_led_set_color(0, 255, 0);
                        sleep_ms(250);
                        rgb_led_off();
                        sleep_ms(250);
                    }
                    
                    sleep_and_check(2000, &button);
                    
                    // ============ PHASE 3: WINK ============
                    
                    // --- PUZZLE 3 HINT ---
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_send_string("P3: Wink Wink!!");
                    speak("Phase 3: Wink Wink!!");
                    rgb_led_set_color(255, 165, 0); // Orange
                    sleep_and_check(3000, &button);
                    
                    // Wait for button press to start Phase 3
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_send_string("Press Button to");
                    speak("Press Button to start Phase 3!");
                    lcd_set_cursor(1, 0);
                    lcd_send_string("start Phase 3!");
                    
                    button_pressed = false;
                    last_blink = 0;
                    led_state = false;
                    
                    while (!button_pressed) {
                        uint32_t now = to_ms_since_boot(get_absolute_time());
                        if (now - last_blink > 500) {
                            led_state = !led_state;
                            if (led_state) {
                                rgb_led_set_color(0, 255, 0);
                            } else {
                                rgb_led_off();
                            }
                            last_blink = now;
                        }
                        
                        if (button_is_pressed(&button)) {
                            rgb_led_set_color(255, 255, 0);
                            sleep_ms(100);
                            while (button_is_pressed(&button)) {
                                sleep_ms(10);
                            }
                            button_pressed = true;
                        }
                        
                        sleep_ms(10);
                    }
                    
                    // Start Phase 3 countdown
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_send_string("Right Wink 5s!");
                    speak("Do Right Wink for 5 seconds!");
                    
                    rgb_led_set_color(0, 0, 255);
                    sleep_ms(500);
                    
                    // --- PHASE 3: 20 SECOND COUNTDOWN ---
                    countdown_start = to_ms_since_boot(get_absolute_time());
                    success = false; // Reset for Phase 3
                    
                    while(1) {
                        check_reset(&button);
                        
                        uint32_t elapsed = to_ms_since_boot(get_absolute_time()) - countdown_start;
                        uint32_t remaining = (countdown_duration - elapsed) / 1000;
                        
                        // Check if time is up
                        if (elapsed >= countdown_duration) {
                            // Time's up - failed Phase 3
                            lcd_clear();
                            lcd_set_cursor(0, 0);
                            lcd_send_string("Hard Luck!");
                            speak("Hard Luck! Try Again!");
                            lcd_set_cursor(1, 0);
                            lcd_send_string("Try Again!");
                            
                            rgb_led_set_color(255, 0, 0);
                            sleep_and_check(3000, &button);
                            
                            // Return to Phase 1 hint and restart
                            goto phase1_start;
                        }
                        
                        // Update countdown display
                        lcd_set_cursor(1, 0);
                        char countdown_str[17];
                        snprintf(countdown_str, sizeof(countdown_str), "Time: %02lus", remaining);
                        lcd_send_string(countdown_str);
                        
                        // Check for incoming data from Python
                        int c = getchar_timeout_us(0);
                        
                        if (c != PICO_ERROR_TIMEOUT) {
                            // Check if it's a digit character (0-9)
                            if (c >= '0' && c <= '9') {
                                last_digit = (char)c;
                                
                                // Flash Green briefly
                                rgb_led_set_color(0, 255, 0);
                                sleep_ms(100);
                                rgb_led_set_color(0, 0, 255);
                            }
                            // Check for "SUCCESS2" message (Phase 3 - Right Wink only)
                            else if (c == 'S') {
                                char buffer[12];
                                buffer[0] = 'S';
                                int buf_idx = 1;
                                
                                for (int i = 0; i < 20 && buf_idx < 11; i++) {
                                    int next_c = getchar_timeout_us(10000);
                                    if (next_c != PICO_ERROR_TIMEOUT) {
                                        buffer[buf_idx++] = (char)next_c;
                                        if (next_c == '\n' || next_c == '\r') break;
                                    }
                                }
                                buffer[buf_idx] = '\0';
                                
                                // Check if it says "SUCCESS2" (for Phase 3 - digit 2 only)
                                if (strstr(buffer, "SUCCESS2") != NULL) {
                                    success = true;
                                    break;
                                }
                            }
                        }
                        
                        sleep_ms(50);
                    }
                    
                    // --- CHECK PHASE 3 SUCCESS ---
                    if (success) {
                        lcd_clear();
                        lcd_set_cursor(0, 0);
                        lcd_send_string("Good Job ");
                        speak("Good Job! All challenges Done! Congratulations!");
                        lcd_set_cursor(1, 0);
                        lcd_send_string("All Done!");
                        
                        // Flash Green - final success!
                        for (int i = 0; i < 10; i++) {
                            rgb_led_set_color(0, 255, 0);
                            sleep_ms(250);
                            rgb_led_off();
                            sleep_ms(250);
                        }
                        
                        // --- MIRROR ROTATION SEQUENCE ---
                        lcd_clear();
                        lcd_set_cursor(0, 0);
                        lcd_send_string("Rotating Mirror");
                        speak("Rotating Mirror");
                        lcd_set_cursor(1, 0);
                        lcd_send_string("Forward...");
                        rgb_led_set_color(0, 255, 255); // Cyan for motor action
                        sleep_and_check(1000, &button);
                        
                        // Rotate mirror 180 degrees forward
                        rotate_mirror_180();
                        
                        lcd_clear();
                        lcd_set_cursor(0, 0);
                        lcd_send_string("Mirror Rotated!");
                        speak("Mirror Rotated! Waiting 7 seconds");
                        lcd_set_cursor(1, 0);
                        lcd_send_string("Wait 7 sec...");
                        rgb_led_set_color(255, 255, 0); // Yellow for waiting
                        sleep_and_check(7000, &button);
                        
                        lcd_clear();
                        lcd_set_cursor(0, 0);
                        lcd_send_string("Rotating Back");
                        speak("Rotating Back");
                        lcd_set_cursor(1, 0);
                        lcd_send_string("to original...");
                        rgb_led_set_color(0, 255, 255); // Cyan for motor action
                        sleep_and_check(1000, &button);
                        
                        // Rotate mirror back 180 degrees (reverse direction)
                        motor_control(170, false);  // false = reverse direction
                        sleep_ms(400);
                        motor_stop();
                        
                        lcd_clear();
                        lcd_set_cursor(0, 0);
                        lcd_send_string("Mirror Reset!");
                        speak("Mirror Reset! System Restarting!");
                        lcd_set_cursor(1, 0);
                        lcd_send_string("Restarting...");
                        rgb_led_set_color(0, 255, 0);
                        sleep_and_check(2000, &button);
                        
                        // Exit all nested blocks and restart from welcome sequence
                        // This breaks out of Phase 3, Phase 2, Phase 1, and password success blocks
                        goto restart_system;
                    }
                }
            }
        }  // Close if (!code_failed) - password success block
        else
        {
            // --- FAILURE: Wrong Code ---
            lcd_set_cursor(0, 0);
            lcd_send_string("Wrong code");
            speak("Wrong code!");

            // Flash Red
            for(int i=0; i<8; i++) {
                rgb_led_set_color(255, 0, 0);
                sleep_and_check(250, &button);
                rgb_led_off();
                sleep_and_check(250, &button);
            }
        }
    }  // Close while(1) loop
}  // Close main()