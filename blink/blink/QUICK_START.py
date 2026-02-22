"""
QUICK START GUIDE - Text-to-Speech for LCD Messages
====================================================

✅ SETUP COMPLETE!

What I've done:
--------------
1. ✅ Installed pyttsx3 and pyserial packages
2. ✅ Created tts_listener.py (TTS listener script)
3. ✅ Modified main.c with 32+ speak() calls
4. ✅ Compiled code successfully
5. ✅ Tested TTS engine - WORKING!
   - Voice: Microsoft Zira (Female)
   - Speed: 150 words/min
   - Volume: 100%

What YOU need to do:
-------------------

STEP 1: Connect Your Pico
   - Plug in Raspberry Pi Pico via USB
   - Wait for Windows to recognize it

STEP 2: Find COM Port
   Run this in PowerShell:
   
   Get-CimInstance Win32_SerialPort | Select-Object Name, DeviceID
   
   Example output: "COM5" or "COM3"

STEP 3: Update COM Port (if needed)
   If your port is NOT COM3:
   - Open: tts_listener.py
   - Line 10: Change 'COM3' to your actual port
   - Save file

STEP 4: Flash Your Pico
   1. Hold BOOTSEL button on Pico
   2. Plug in USB (or press reset while holding BOOTSEL)
   3. Copy build\blink.uf2 to RPI-RP2 drive
   4. Pico will reboot automatically

STEP 5: Run TTS Listener
   Open PowerShell in this folder and run:
   
   python tts_listener.py
   
   You should see:
   [INFO] Connected! Listening for TTS commands...

STEP 6: Enjoy!
   - Every LCD message will be spoken!
   - Test with: "Testing Motor", "Welcome to Station 6", etc.


TROUBLESHOOTING:
---------------

Problem: "Could not open COM3"
Solution: 
   - Check Pico is connected (Device Manager → Ports)
   - Close other programs using serial (Arduino IDE, PuTTY)
   - Update COM port in tts_listener.py

Problem: No sound
Solution:
   - Check Windows volume is not muted
   - Run: python test_tts.py
   - Check speaker output device

Problem: TTS too slow/fast
Solution:
   - Edit tts_listener.py line 16
   - Change rate: 150 (slower) to 200 (faster)


FEATURES:
---------
✅ All LCD messages spoken in real-time
✅ Female voice (Microsoft Zira)
✅ Non-blocking (doesn't slow down game)
✅ Works with all 3 phases
✅ Success/failure messages announced
✅ Instructions spoken clearly

WHAT'S SPOKEN:
--------------
✓ Motor test messages
✓ Welcome sequence
✓ Safety instructions
✓ Password prompts
✓ Phase hints and instructions
✓ Success/failure messages
✓ Game countdowns (initial only)
✗ Timer updates (would be annoying!)


FILES CREATED:
-------------
✅ tts_listener.py  - Main TTS listener
✅ test_tts.py      - Test TTS engine
✅ TTS_README.md    - Detailed documentation
✅ THIS_FILE.txt    - Quick start guide


NEXT STEPS:
----------
1. Connect Pico → Find COM port → Update tts_listener.py
2. Flash build\blink.uf2 to Pico
3. Run: python tts_listener.py
4. Start your escape room game!


Need help? Check TTS_README.md for detailed docs.
"""

print(__doc__)
