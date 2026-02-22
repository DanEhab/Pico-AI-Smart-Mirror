#!/usr/bin/env python3
import sys
print(f"Python version: {sys.version}")

try:
    print("Importing numpy...")
    import numpy as np
    print(f"✓ Numpy {np.__version__} imported successfully")
except Exception as e:
    print(f"✗ Failed to import numpy: {e}")
    sys.exit(1)

try:
    print("Importing cv2...")
    import cv2
    print(f"✓ OpenCV {cv2.__version__} imported successfully")
except Exception as e:
    print(f"✗ Failed to import cv2: {e}")
    sys.exit(1)

try:
    print("Importing mediapipe...")
    import mediapipe as mp
    print(f"✓ MediaPipe {mp.__version__} imported successfully")
except Exception as e:
    print(f"✗ Failed to import mediapipe: {e}")
    sys.exit(1)

print("\n✓ All imports successful!")
print("\nTesting camera access...")
try:
    cap = cv2.VideoCapture(0)
    if cap.isOpened():
        print("✓ Camera opened successfully")
        ret, frame = cap.read()
        if ret:
            print(f"✓ Frame captured: {frame.shape}")
        else:
            print("✗ Failed to capture frame")
        cap.release()
    else:
        print("✗ Failed to open camera")
except Exception as e:
    print(f"✗ Camera error: {e}")

input("\nPress Enter to exit...")
