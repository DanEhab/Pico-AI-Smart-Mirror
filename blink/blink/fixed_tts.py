import serial
import serial.tools.list_ports
import pyttsx3
import threading
import queue
import time

# Create a queue to hold messages safely
speech_queue = queue.Queue()

def find_pico_port():
    """Find the Raspberry Pi Pico COM port automatically"""
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        # Pico usually shows up as "USB Serial Device"
        if "USB" in port.description or "Serial" in port.description:
            return port.device
    return None

def tts_worker():
    """
    Worker thread that re-initializes the engine for every sentence.
    This prevents the 'One and Done' silence bug.
    """
    print("✅ TTS Worker Started.")

    while True:
        # 1. Wait for a message
        text_to_speak = speech_queue.get()
        
        if text_to_speak is None:
            break 
        
        try:
            # --- THE FIX: Initialize Engine INSIDE the loop ---
            # This forces a fresh connection to Windows Audio every time
            engine = pyttsx3.init()
            engine.setProperty('rate', 150)
            engine.setProperty('volume', 1.0)
            
            # Optional: Set voice again here if you want specific gender
            # voices = engine.getProperty('voices')
            # if len(voices) > 1: engine.setProperty('voice', voices[1].id)

            print(f"🔊 Speaking: {text_to_speak}")
            engine.say(text_to_speak)
            engine.runAndWait()
            
            # 2. Explicitly stop/close the engine
            engine.stop()
            del engine # Delete the object to free memory
            
        except Exception as e:
            print(f"❌ Error in speech loop: {e}")
        
        # 3. Mark task as done and wait a tiny bit to let audio clear
        speech_queue.task_done()
        time.sleep(0.2)

def main():
    print("=" * 60)
    print("  SMART MIRROR TTS SERVER (Fixed)")
    print("=" * 60)

    # 1. Start the TTS Worker Thread
    worker_thread = threading.Thread(target=tts_worker, daemon=True)
    worker_thread.start()

    # 2. Find Pico
    print("\n⏳ Searching for Pico...")
    port = None
    while port is None:
        port = find_pico_port()
        if port:
            break
        time.sleep(1)
        print("   Scanning...", end='\r')
    
    print(f"\n✅ Found device on {port}")

    # 3. Serial Loop
    try:
        ser = serial.Serial(port, 115200, timeout=1)
        print("✅ Serial Connected! Listening...")

        while True:
            if ser.in_waiting > 0:
                try:
                    # Read line and strip whitespace
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    
                    if line.startswith("TTS:"):
                        # Put message in queue (Non-blocking)
                        message = line[4:].strip()
                        if message:
                            speech_queue.put(message)
                    
                    elif line == "START":
                        print("🚀 Triggering AI Sequence")
                    
                    elif line:
                        print(f"📨 Raw: {line}")

                except UnicodeDecodeError:
                    pass 
            
            time.sleep(0.01)

    except serial.SerialException as e:
        print(f"\n❌ Serial Disconnected: {e}")

if __name__ == "__main__":
    main()