# Text-to-Speech Setup for LCD Messages

## 🎯 What This Does

Every message displayed on the LCD screen will be spoken aloud through your laptop speakers using Windows Text-to-Speech!

---

## 📋 Setup Instructions

### 1. Install Python Package

Open PowerShell or Command Prompt and run:

```powershell
pip install pyttsx3 pyserial
```

### 2. Check Your COM Port

1. Open **Device Manager** (Win + X → Device Manager)
2. Expand **Ports (COM & LPT)**
3. Find your Raspberry Pi Pico (usually shows as "USB Serial Device")
4. Note the COM port number (e.g., COM3, COM5)

If it's **NOT COM3**, edit `tts_listener.py` line 10:
```python
SERIAL_PORT = 'COM5'  # Change to your actual port
```

### 3. Run the TTS Listener

In the terminal, navigate to your project folder and run:

```powershell
cd "d:\Daniel\GUC\Semester 7\Embedded\blink\blink"
python tts_listener.py
```

You should see:
```
==================================================
  Raspberry Pi Pico TTS Listener
==================================================

[INFO] Connecting to COM3 at 115200 baud...
[INFO] Connected! Listening for TTS commands...
```

### 4. Flash Your Pico

1. Flash `build\blink.uf2` to your Pico
2. The TTS listener will automatically start speaking LCD messages!

---

## 🔧 How It Works

### In `main.c`:
Every time text is displayed on the LCD, a `speak()` function sends:
```c
speak("Welcome to Station 6");
```

This sends over USB serial:
```
TTS:Welcome to Station 6
```

### In `tts_listener.py`:
The Python script:
1. Listens for lines starting with `TTS:`
2. Extracts the text after `TTS:`
3. Uses Windows TTS engine to speak it
4. Runs speech in a separate thread (doesn't block serial communication)

---

## 🎛️ Customization

### Change Voice Speed

Edit `tts_listener.py` line 16:
```python
engine.setProperty('rate', 150)  # Default 200, slower = 100, faster = 250
```

### Change Voice (Male/Female)

Edit `tts_listener.py` line 22:
```python
engine.setProperty('voice', voices[0].id)  # 0 = male, 1 = female (usually)
```

### Change Volume

Edit `tts_listener.py` line 17:
```python
engine.setProperty('volume', 1.0)  # Range: 0.0 (mute) to 1.0 (max)
```

---

## 🐛 Troubleshooting

### "Could not open COM3"
- Make sure Pico is connected via USB
- Check correct COM port in Device Manager
- Close any other programs using the serial port (Arduino IDE, PuTTY, etc.)

### No speech but listener is running
- Check Windows sound settings (not muted)
- Verify `pyttsx3` is installed: `pip list | findstr pyttsx3`
- Try running: `python -c "import pyttsx3; e=pyttsx3.init(); e.say('test'); e.runAndWait()"`

### Speech is too slow/fast
- Adjust the `rate` property in `tts_listener.py` (line 16)

### Wrong COM port
- Edit `SERIAL_PORT` in `tts_listener.py` (line 10)

---

## 📝 Notes

- The TTS listener must be running **BEFORE** you power on/reset the Pico
- Speech happens in real-time as messages appear on LCD
- Multiple messages spoken quickly may overlap slightly
- Countdown timers are NOT spoken (would be too annoying!)

---

## ▶️ Quick Start Command

```powershell
python tts_listener.py
```

Then flash and run your Pico code!

---

Enjoy your talking escape room station! 🎙️🔊
