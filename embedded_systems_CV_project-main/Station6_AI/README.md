# Station6 AI - Facial Expression Recognition System

A real-time facial expression detection system that interfaces with Raspberry Pi Pico for multi-phase challenge completion. Uses MediaPipe Face Mesh to detect 10 different facial expressions mapped to digits 0-9.

## Features

- **Real-time facial expression detection** for 10 expressions (digits 0-9)
- **Serial communication** with Raspberry Pi Pico
- **Multi-phase challenge system** with automatic SUCCESS signaling
- **Text-to-Speech (TTS)** support for English and Arabic
- **Background music playback** triggered by Pico commands
- **Expression hold tracking** with 10-second accumulation
- **Automatic retry handling** with expression tracking reset

## Expression Mapping

| Digit | Expression |
|-------|-----------|
| 0 | Neutral/Relaxed Face |
| 1 | Smile/Laughing ⭐ (Phase 1) |
| 2 | Right Eye Blink/Wink ⭐ (Phase 3) |
| 3 | Sad/Frown |
| 4 | Raise Right Eyebrow |
| 5 | Open Mouth (O-Shape) |
| 6 | Surprise (Wide Eyes) ⭐ (Phase 2) |
| 7 | Pout/Duck Face |
| 8 | Left Eye Blink/Wink |
| 9 | Right Cheek Puff |

⭐ = Phase-critical expressions (trigger SUCCESS messages when held for 10 seconds)

## System Requirements

### Software Requirements

- **Python 3.11.x** (Required - Python 3.14+ has compatibility issues with numpy)
- **Windows OS** (tested on Windows 10/11)
- **Visual C++ Redistributable** (required for MediaPipe)

### Hardware Requirements

- **Webcam** (any USB or built-in camera)
- **Raspberry Pi Pico** (connected via USB serial)
- **COM Port** (default: COM3 at 115200 baud)

## Installation

### Step 1: Install Python 3.11

1. Download Python 3.11.9 from [python.org](https://www.python.org/downloads/)
2. During installation, check "Add Python to PATH"
3. Verify installation:
   ```powershell
   py -3.11 --version
   ```

### Step 2: Install Visual C++ Redistributable

MediaPipe requires Visual C++ runtime libraries:

1. Download from: https://aka.ms/vs/17/release/vc_redist.x64.exe
2. Run installer and follow prompts
3. Restart your computer if prompted

### Step 3: Install Python Dependencies

Open PowerShell in the `Station6_AI` directory and run:

```powershell
py -3.11 -m pip install -r requirements.txt
```

This installs:
- `opencv-python` - Camera capture and video processing
- `mediapipe` - Face mesh landmark detection
- `numpy` - Mathematical operations
- `pyserial` - Serial communication with Pico
- `pyttsx3` - Text-to-speech engine
- `pygame` - Audio playback

### Step 4: Download Pre-trained Model

The MediaPipe Face Landmarker model is required for facial detection.

1. **Download the model file:**
   - URL: https://storage.googleapis.com/mediapipe-models/face_landmarker/face_landmarker/float16/latest/face_landmarker.task
   - File size: ~10 MB

2. **Place the file:**
   - Save as: `src/face_landmarker.task`
   - Full path: `Station6_AI/src/face_landmarker.task`

3. **Verify:**
   ```
   Station6_AI/
   ├── src/
   │   ├── face_landmarker.task  ✅ (should exist)
   │   └── main.py
   ```

**Quick Download (PowerShell):**
```powershell
# Navigate to src directory
cd "D:\Daniel\GUC\Semester 7\Embedded\embedded_systems_CV_project-main\Station6_AI\src"

# Download model file
Invoke-WebRequest -Uri "https://storage.googleapis.com/mediapipe-models/face_landmarker/face_landmarker/float16/latest/face_landmarker.task" -OutFile "face_landmarker.task"
```

### Step 5: Add Music File (Optional)

For background music playback during Phase 3 completion:

1. Add `song.mp3` to the `Station6_AI/` directory
2. Duration: Any length (will play for 17 seconds then fade out)
3. If missing, music feature will be skipped with error message

## Configuration

### Serial Port Configuration

Default settings in `serial_sender.py`:
- **Port:** COM3
- **Baud Rate:** 115200

To change COM port:
1. Open `serial_sender.py`
2. Modify line: `def __init__(self, port='COM3', baudrate=115200):`
3. Change `'COM3'` to your port (e.g., `'COM4'`)

**Find your Pico's COM port:**
```powershell
# List all COM ports
Get-PnpDevice -PresentOnly | Where-Object { $_.InstanceId -match '^USB' }
```

### Camera Configuration

Default: `cv2.VideoCapture(0)` - uses first available camera

To use a different camera:
1. Open `src/main.py`
2. Find line: `cap = cv2.VideoCapture(0)`
3. Change `0` to camera index (1, 2, etc.)

### Expression Detection Thresholds

Located in `src/main.py` `detect_expression()` function:

```python
EYE_CLOSED_THRESHOLD = 0.15      # Lower = stricter blink detection
MOUTH_OPEN_THRESHOLD = 0.5       # Higher = wider mouth needed
SMILE_THRESHOLD = 3.0            # Pixels above neutral for smile
FROWN_THRESHOLD = -2.0           # Pixels below neutral for frown
EYEBROW_RAISE_THRESHOLD = -35    # Distance for eyebrow raise
CHEEK_PUFF_THRESHOLD = 1.15      # 15% face width increase
```

Adjust these values if expressions are too sensitive/insensitive.

## Running the Application

### Method 1: Direct Execution

```powershell
py -3.11 "d:/Daniel/GUC/Semester 7/Embedded/embedded_systems_CV_project-main/Station6_AI/src/main.py"
```

### Method 2: From Station6_AI Directory

```powershell
cd "D:\Daniel\GUC\Semester 7\Embedded\embedded_systems_CV_project-main\Station6_AI"
py -3.11 src/main.py
```

## Usage Workflow

1. **Connect Pico:** Plug in Raspberry Pi Pico via USB
2. **Run Python script:** Camera window will NOT open yet
3. **Enter password on Pico:** Follow Pico LCD instructions
4. **Press button on Pico:** Sends START signal to Python
5. **Camera opens:** Python begins facial expression detection
6. **Perform expressions:**
   - **Phase 1:** Hold smile (digit 1) for 10 seconds → Python sends `SUCCESS1\n`
   - **Phase 2:** Hold surprise (digit 6) for 10 seconds → Python sends `SUCCESS6\n`
   - **Phase 3:** Hold right wink (digit 2) for 10 seconds → Python sends `SUCCESS2\n`
7. **TTS & Music:** Pico can send TTS and music commands back to Python
8. **Retry:** If phase fails, Pico sends START again, tracking resets automatically

### Keyboard Controls

- **Q:** Quit application (closes camera and serial connection)

## Serial Protocol

### Messages from Pico → Python

| Message | Action |
|---------|--------|
| `START\n` | Initialize/reset camera and expression tracking |
| `TTS:Hello World\n` | Speak "Hello World" via TTS (auto-detects English/Arabic) |
| `MUSIC:\n` | Play `song.mp3` for 17 seconds |

### Messages from Python → Pico

| Message | Trigger |
|---------|---------|
| `0` - `9` | Single digit on expression change |
| `SUCCESS1\n` | Smile held for 10 seconds |
| `SUCCESS2\n` | Right wink held for 10 seconds |
| `SUCCESS6\n` | Surprise held for 10 seconds |

## Troubleshooting

### Camera Not Opening

**Error:** `Error: Could not open camera`

**Solutions:**
1. Check if another application is using the camera
2. Try different camera index: `cv2.VideoCapture(1)`
3. Grant camera permissions in Windows Settings
4. Restart computer

### Serial Connection Failed

**Error:** `Failed to connect to Pico`

**Solutions:**
1. Verify Pico is plugged in via USB
2. Check COM port in Device Manager
3. Update `serial_sender.py` with correct port
4. Try different USB cable/port
5. Install Pico USB drivers if needed

### MediaPipe Import Error

**Error:** `DLL load failed while importing _framework_bindings`

**Solutions:**
1. Install Visual C++ Redistributable (see Step 2)
2. Restart computer after installation
3. Reinstall mediapipe: `py -3.11 -m pip install --upgrade mediapipe`

### Numpy Compatibility Issues

**Error:** `Numpy built with MINGW-W64... CRASHES ARE TO BE EXPECTED`

**Solutions:**
1. Use Python 3.11 (NOT 3.14+)
2. Install specific numpy version: `py -3.11 -m pip install "numpy>=1.26.4,<2.0"`

### Expression Detection Issues

**Problem:** Wrong expressions detected or too sensitive

**Solutions:**
1. Ensure good lighting on face
2. Look directly at camera (mirror view)
3. Adjust thresholds in `detect_expression()` function
4. Increase CHEEK_PUFF_THRESHOLD if cheek puff detected incorrectly

### TTS Not Working

**Problem:** No speech output

**Solutions:**
1. Check Windows volume/mute status
2. Verify pyttsx3 installation: `py -3.11 -m pip install --upgrade pyttsx3`
3. Test voices: Run `pyttsx3-tools` in terminal
4. Check thread exceptions in console output

### Music Not Playing

**Problem:** Music doesn't play or error shown

**Solutions:**
1. Ensure `song.mp3` exists in `Station6_AI/` directory
2. Check file format (must be MP3)
3. Verify pygame installation: `py -3.11 -m pip install --upgrade pygame`
4. Check audio device is working

## Project Structure

```
Station6_AI/
├── src/
│   ├── main.py                  # Main application
│   ├── face_landmarker.task     # MediaPipe model (download required)
│   └── __pycache__/             # Python cache
├── serial_sender.py             # Serial communication wrapper
├── requirements.txt             # Python dependencies
├── README.md                    # This file
├── test_cam.py                  # Camera test utility
└── song.mp3                     # Background music (optional)
```

## Technical Details

### Face Landmark Detection

- **Model:** MediaPipe Face Mesh
- **Landmarks:** 468 facial landmarks per face
- **Detection confidence:** 0.5 minimum
- **Tracking confidence:** 0.5 minimum
- **Max faces:** 1 (performance optimization)
- **Refinements:** Enabled (includes iris landmarks)

### Expression Detection Algorithm

Expressions are detected using geometric calculations:

1. **Eye Aspect Ratio (EAR):** Vertical/horizontal eye distance for blinks
2. **Mouth Aspect Ratio (MAR):** Vertical/horizontal mouth distance for open mouth
3. **Smile Score:** Mouth corner height relative to center
4. **Cheek Puff Ratio:** Face width at cheek level vs reference width
5. **Eyebrow Distance:** Eyebrow position relative to eye

### Performance

- **Frame Rate:** 30-60 FPS (depends on CPU/GPU)
- **Latency:** <50ms expression detection
- **CPU Usage:** 20-40% on modern CPUs
- **Memory:** ~200-300 MB

## Version History

- **v3.0:** Added Phase 3 support (right wink), TTS, music playback
- **v2.0:** Added Phase 2 support (surprise), expression tracking reset
- **v1.0:** Initial release with Phase 1 (smile), serial communication

## Credits

- **MediaPipe:** Google's Face Mesh solution
- **OpenCV:** Camera capture and image processing
- **pyttsx3:** Cross-platform text-to-speech

## License

Educational project for embedded systems course - GUC Semester 7

---

**Need help?** Check console output for error messages and refer to Troubleshooting section above.
