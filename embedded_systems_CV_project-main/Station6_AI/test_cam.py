import cv2
import time

print("--- Camera Test Start ---")

# Try index 0, then 1 (sometimes 0 is occupied)
for index in [0, 1]:
    print(f"Testing camera index {index}...")
    cap = cv2.VideoCapture(index)

    if cap.isOpened():
        print(f"SUCCESS: Camera index {index} is working!")
        ret, frame = cap.read()
        if ret:
            print(f"SUCCESS: Successfully grabbed a frame of size {frame.shape}")
        else:
            print("WARNING: Camera opened but failed to grab frame.")
        cap.release()
        break # Stop if we found a working camera
    else:
        print(f"FAIL: Could not open camera index {index}.")

print("--- Camera Test End ---")