"""
Text-to-Speech Listener for Raspberry Pi Pico LCD Messages
Listens for TTS: commands over serial and speaks them using Windows TTS
"""

import serial
import pyttsx3
import threading
import time

# Configure serial port (adjust COM port if needed)
SERIAL_PORT = 'COM3'  # Change to your Pico's COM port
BAUD_RATE = 115200

def speak_text(text):
    """Speak text using Windows TTS in a separate thread"""
    try:
        engine = pyttsx3.init()
        engine.setProperty('rate', 150)  # Speed (default 200)
        engine.setProperty('volume', 1.0)  # Volume (0.0 to 1.0)
        
        # Optional: Set voice (0 = male, 1 = female typically)
        voices = engine.getProperty('voices')
        if len(voices) > 1:
            engine.setProperty('voice', voices[1].id)  # Female voice
        
        print(f"[TTS] Speaking: {text}")
        engine.say(text)
        engine.runAndWait()
    except Exception as e:
        print(f"[TTS ERROR] {e}")

def main():
    print(f"[INFO] Connecting to {SERIAL_PORT} at {BAUD_RATE} baud...")
    
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Wait for connection to stabilize
        print(f"[INFO] Connected! Listening for TTS commands...")
        
        while True:
            try:
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    
                    if line.startswith("TTS:"):
                        # Extract text after "TTS:" prefix
                        text = line[4:].strip()
                        
                        if text:
                            # Speak in separate thread to avoid blocking serial reading
                            speech_thread = threading.Thread(target=speak_text, args=(text,))
                            speech_thread.daemon = True
                            speech_thread.start()
                    else:
                        # Print other serial messages for debugging
                        if line:
                            print(f"[SERIAL] {line}")
                
                time.sleep(0.01)  # Small delay to prevent CPU overload
                
            except UnicodeDecodeError:
                pass  # Ignore decode errors
            except KeyboardInterrupt:
                print("\n[INFO] Stopping TTS listener...")
                break
                
    except serial.SerialException as e:
        print(f"[ERROR] Could not open {SERIAL_PORT}: {e}")
        print("[INFO] Make sure:")
        print("  1. Pico is connected via USB")
        print("  2. Correct COM port is selected")
        print("  3. No other program is using the serial port")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("[INFO] Serial port closed")

if __name__ == "__main__":
    print("=" * 50)
    print("  Raspberry Pi Pico TTS Listener")
    print("=" * 50)
    print()
    
    # Check if pyttsx3 is installed
    try:
        import pyttsx3
    except ImportError:
        print("[ERROR] pyttsx3 not installed!")
        print("[FIX] Run: pip install pyttsx3")
        exit(1)
    
    main()
