"""
Auto-Start TTS Listener with COM Port Detection
Waits for Pico to be plugged in, then starts speaking LCD messages
"""

import serial
import serial.tools.list_ports
import pyttsx3
import threading
import time

def find_pico_port():
    """Find the Raspberry Pi Pico COM port"""
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        # Return first available COM port (usually the Pico)
        return port.device
    return None

def speak_text(text):
    """Speak text using Windows TTS in a separate thread"""
    try:
        engine = pyttsx3.init()
        voices = engine.getProperty('voices')
        if len(voices) > 1:
            engine.setProperty('voice', voices[1].id)  # Female voice
        engine.setProperty('rate', 150)
        engine.setProperty('volume', 1.0)
        
        print(f"🔊 Speaking: {text}")
        engine.say(text)
        engine.runAndWait()
    except Exception as e:
        print(f"[TTS ERROR] {e}")

def main():
    print("=" * 60)
    print("  TTS LISTENER - Waiting for Raspberry Pi Pico...")
    print("=" * 60)
    print()
    print("⏳ Waiting for Pico to be plugged in...")
    print("   (Flash code to Pico first, then plug in USB)")
    print()
    
    # Initialize TTS engine once
    try:
        engine = pyttsx3.init()
        voices = engine.getProperty('voices')
        engine.setProperty('voice', voices[1].id)  # Female voice
        engine.setProperty('rate', 150)
        print(f"🎤 Voice engine ready: {voices[1].name}")
    except Exception as e:
        print(f"❌ TTS Engine error: {e}")
        input("Press Enter to exit...")
        return
    
    # Wait for Pico to be connected
    port = None
    dots = 0
    while port is None:
        port = find_pico_port()
        if port is None:
            dots = (dots + 1) % 4
            print(f"⏳ Still waiting for Pico{'.' * dots}   ", end='\r')
            time.sleep(0.5)
    
    print()  # New line after waiting
    print(f"✅ Found Pico on {port}!")
    print()
    
    # Try to connect
    try:
        ser = serial.Serial(port, 115200, timeout=1)
        time.sleep(2)  # Wait for serial to initialize
        print(f"✅ Connected to {port} at 115200 baud")
        print("✅ Listening for LCD messages...")
        print("🔊 All LCD text will be spoken!")
        print("   (Press Ctrl+C to stop)")
        print()
        print("-" * 60)
        
        while True:
            if ser.in_waiting > 0:
                try:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    
                    # Check if it's a TTS command
                    if line.startswith("TTS:"):
                        message = line[4:]  # Remove "TTS:" prefix
                        # Speak in separate thread to avoid blocking
                        thread = threading.Thread(target=speak_text, args=(message,))
                        thread.daemon = True
                        thread.start()
                    elif line and not line.startswith("START"):
                        # Print other messages for debugging (skip START signals)
                        print(f"📨 {line}")
                        
                except UnicodeDecodeError:
                    pass  # Ignore decode errors
                    
            time.sleep(0.01)  # Small delay to prevent CPU spinning
            
    except serial.SerialException as e:
        print(f"\n❌ Serial Error: {e}")
        print()
        print("💡 Make sure:")
        print("   1. Pico is plugged in and running code")
        print("   2. No other program is using the serial port")
        print("   3. Pico is NOT in BOOTSEL mode")
        print()
        input("Press Enter to exit...")
    except KeyboardInterrupt:
        print("\n\n👋 TTS Listener stopped by user")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("🔌 Serial port closed")

if __name__ == "__main__":
    main()
