#!/usr/bin/env python3
"""
Facial Expression Detection for Digits 0-9
Displays live camera feed with detected expression
Integrates with Raspberry Pi Pico via serial communication
"""

import warnings
warnings.filterwarnings('ignore')

import cv2
import mediapipe as mp
import numpy as np
import sys
import os
import time  # <--- MAKE SURE THIS IS HERE AT THE TOP
# --- ADD THESE IMPORTS ---
import threading
import queue
import pyttsx3
import pygame

# --- ADD THIS HERE (Global Init) ---
try:
    pygame.mixer.init()
except:
    print("Audio init failed or already initialized")
# -----------------------------------


# --- ADD THIS QUEUE DEFINITION ---
speech_queue = queue.Queue()

# Add parent directory to path to import serial_sender
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from serial_sender import PicoSender

# Initialize MediaPipe Face Mesh
mp_face_mesh = mp.solutions.face_mesh
mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles

# Expression mapping to digits
EXPRESSIONS = {
    0: "Neutral/Relaxed Face",
    1: "Smile/Laughing",
    2: "Right Eye Blink/Wink",
    3: "Sad/Frown",
    4: "Raise Right Eyebrow",
    5: "Open Mouth (O-Shape)",
    6: "Surprise (Wide Eyes)",
    7: "Pout/Duck Face",
    8: "Left Eye Blink/Wink",
    9: "Right Cheek Puff"
}

# Key landmark indices for expression detection
RIGHT_EYE = [33, 160, 158, 133, 153, 144]
LEFT_EYE = [362, 385, 387, 263, 373, 380]
MOUTH_OUTER = [61, 146, 91, 181, 84, 17, 314, 405, 321, 375, 291, 308]
LIPS = [61, 185, 40, 39, 37, 0, 267, 269, 270, 409, 291]
RIGHT_EYEBROW = [70, 63, 105, 66, 107]
LEFT_EYEBROW = [336, 296, 334, 293, 300]


def calculate_eye_aspect_ratio(landmarks, eye_indices, frame_shape):
    """Calculate Eye Aspect Ratio (EAR) to detect blinks"""
    h, w = frame_shape[:2]
    
    eye_points = []
    for idx in eye_indices:
        landmark = landmarks.landmark[idx]
        eye_points.append([landmark.x * w, landmark.y * h])
    
    eye_points = np.array(eye_points)
    
    # Vertical eye distances
    A = np.linalg.norm(eye_points[1] - eye_points[5])
    B = np.linalg.norm(eye_points[2] - eye_points[4])
    # Horizontal eye distance
    C = np.linalg.norm(eye_points[0] - eye_points[3])
    
    ear = (A + B) / (2.0 * C)
    return ear


def calculate_mouth_aspect_ratio(landmarks, frame_shape):
    """Calculate Mouth Aspect Ratio (MAR) to detect open mouth"""
    h, w = frame_shape[:2]
    
    # Get multiple lip landmarks for better accuracy
    upper_lip_top = landmarks.landmark[13]
    lower_lip_bottom = landmarks.landmark[14]
    left_mouth = landmarks.landmark[61]
    right_mouth = landmarks.landmark[291]
    
    # Calculate distances
    vertical_dist = abs(upper_lip_top.y - lower_lip_bottom.y) * h
    horizontal_dist = abs(left_mouth.x - right_mouth.x) * w
    
    if horizontal_dist == 0:
        return 0
    
    mar = vertical_dist / horizontal_dist
    return mar


def calculate_smile_score(landmarks, frame_shape):
    """
    Calculate smile score using multiple mouth landmarks
    Returns positive value for smile, negative for frown
    """
    h, w = frame_shape[:2]
    
    # Mouth corners
    left_corner = landmarks.landmark[61]   # Left corner
    right_corner = landmarks.landmark[291]  # Right corner
    
    # Mouth center points
    upper_lip_center = landmarks.landmark[13]
    lower_lip_center = landmarks.landmark[14]
    
    # Calculate average mouth corner height
    avg_corner_y = (left_corner.y + right_corner.y) / 2
    
    # Calculate center lip height
    avg_center_y = (upper_lip_center.y + lower_lip_center.y) / 2
    
    # Smile score: corners higher than center = positive (smile)
    # corners lower than center = negative (frown)
    smile_score = (avg_center_y - avg_corner_y) * h
    
    return smile_score


def calculate_cheek_puff(landmarks, frame_shape):
    """
    Detect cheek puff by measuring face width at cheek level
    Returns a score indicating cheek inflation
    """
    h, w = frame_shape[:2]
    
    # Get face width at different levels
    # Cheek landmarks (mid-face width)
    left_cheek = landmarks.landmark[234]   # Left cheek
    right_cheek = landmarks.landmark[454]  # Right cheek
    
    # Jaw width (lower face)
    left_jaw = landmarks.landmark[172]
    right_jaw = landmarks.landmark[397]
    
    # Temple width (upper face reference)
    left_temple = landmarks.landmark[127]
    right_temple = landmarks.landmark[356]
    
    # Calculate widths
    cheek_width = abs(left_cheek.x - right_cheek.x) * w
    jaw_width = abs(left_jaw.x - right_jaw.x) * w
    temple_width = abs(left_temple.x - right_temple.x) * w
    
    # Cheek puff ratio: cheek width compared to average of jaw and temple
    avg_reference_width = (jaw_width + temple_width) / 2
    
    if avg_reference_width == 0:
        return 0
    
    cheek_ratio = cheek_width / avg_reference_width
    
    return cheek_ratio


def calculate_eyebrow_raise(landmarks, eyebrow_indices, eye_indices, frame_shape):
    """Calculate eyebrow raise relative to eye position"""
    h, w = frame_shape[:2]
    
    # Average eyebrow position
    eyebrow_y = np.mean([landmarks.landmark[idx].y for idx in eyebrow_indices])
    
    # Average eye position
    eye_y = np.mean([landmarks.landmark[idx].y for idx in eye_indices])
    
    # Distance (negative means eyebrow is above eye, which is normal)
    distance = (eyebrow_y - eye_y) * h
    
    return distance


def detect_expression(landmarks, frame_shape):
    """
    Detect facial expression and return corresponding digit (0-9)
    """
    if not landmarks:
        return 0, EXPRESSIONS[0]
    
    # Calculate features
    right_ear = calculate_eye_aspect_ratio(landmarks, RIGHT_EYE, frame_shape)
    left_ear = calculate_eye_aspect_ratio(landmarks, LEFT_EYE, frame_shape)
    mar = calculate_mouth_aspect_ratio(landmarks, frame_shape)
    smile_score = calculate_smile_score(landmarks, frame_shape)
    cheek_puff_ratio = calculate_cheek_puff(landmarks, frame_shape)
    
    # Eyebrow raises
    right_brow_dist = calculate_eyebrow_raise(landmarks, RIGHT_EYEBROW, RIGHT_EYE, frame_shape)
    left_brow_dist = calculate_eyebrow_raise(landmarks, LEFT_EYEBROW, LEFT_EYE, frame_shape)
    
    # Thresholds (tuned for better detection)
    EYE_CLOSED_THRESHOLD = 0.15
    MOUTH_OPEN_THRESHOLD = 0.5
    SMILE_THRESHOLD = 3.0  # Pixels above center
    FROWN_THRESHOLD = -2.0  # Pixels below center
    EYEBROW_RAISE_THRESHOLD = -35
    CHEEK_PUFF_THRESHOLD = 1.15  # 15% wider than average (increased sensitivity)
    
    # Expression detection logic (priority order matters)
    
    # 2: Right Eye Blink/Wink (left eye closed, right open - flipped for mirror)
    if left_ear < EYE_CLOSED_THRESHOLD and right_ear > EYE_CLOSED_THRESHOLD + 0.05:
        return 2, EXPRESSIONS[2]
    
    # 8: Left Eye Blink/Wink (right eye closed, left open - flipped for mirror)
    if right_ear < EYE_CLOSED_THRESHOLD and left_ear > EYE_CLOSED_THRESHOLD + 0.05:
        return 8, EXPRESSIONS[8]
    
    # 9: Right Cheek Puff (face wider at cheeks) - moved after other expressions
    # Only detect if mouth is not doing other expressions
    if cheek_puff_ratio > CHEEK_PUFF_THRESHOLD and mar < 0.3 and abs(smile_score) < 2.0:
        return 9, EXPRESSIONS[9]
    
    # 6: Surprise (Wide Eyes) - both eyes very open
    if right_ear > 0.30 and left_ear > 0.30:
        return 6, EXPRESSIONS[6]
    
    # 5: Open Mouth (O-Shape) - wide open
    if mar > MOUTH_OPEN_THRESHOLD:
        return 5, EXPRESSIONS[5]
    
    # 1: Smile/Laughing (mouth corners up significantly)
    if smile_score > SMILE_THRESHOLD:
        return 1, EXPRESSIONS[1]
    
    # 3: Sad/Frown (mouth corners down)
    if smile_score < FROWN_THRESHOLD:
        return 3, EXPRESSIONS[3]
    
    # 4: Raise Right Eyebrow
    if right_brow_dist < EYEBROW_RAISE_THRESHOLD and left_brow_dist > EYEBROW_RAISE_THRESHOLD - 10:
        return 4, EXPRESSIONS[4]
    
    # 7: Pout/Duck Face (lips forward, moderate mouth opening)
    if mar > 0.2 and mar < 0.45 and abs(smile_score) < 2.0:
        return 7, EXPRESSIONS[7]
    
    # 0: Neutral/Relaxed Face (default)
    return 0, EXPRESSIONS[0]


# --- ADD THIS FUNCTION ---
def tts_worker():
    """
    Worker thread that detects language (English vs Arabic) 
    and switches voices automatically for each sentence.
    """
    print("✅ TTS Worker Started.")
    
    while True:
        text_to_speak = speech_queue.get()
        if text_to_speak is None: break 
        
        try:
            # Initialize engine FRESH every time to prevent freezing
            engine = pyttsx3.init()
            engine.setProperty('rate', 150)
            engine.setProperty('volume', 1.0)
            
            # 1. FIND VOICE IDs
            voices = engine.getProperty('voices')
            arabic_id = None
            english_id = None
            
            for v in voices:
                # Look for Arabic
                if "Arabic" in v.name or "Hoda" in v.name or "Naayf" in v.name:
                    arabic_id = v.id
                # Look for English (Zira is standard US Female, David is Male)
                if "English" in v.name or "Zira" in v.name or "David" in v.name:
                    english_id = v.id
            
            # Fallback: If no English specific found, use the first one (usually system default)
            if english_id is None and len(voices) > 0:
                english_id = voices[0].id

            # 2. DETECT LANGUAGE
            # Check if the text contains any Arabic characters (Unicode range 0600-06FF)
            is_arabic_text = any('\u0600' <= char <= '\u06FF' for char in text_to_speak)
            
            # 3. SELECT VOICE
            if is_arabic_text and arabic_id:
                engine.setProperty('voice', arabic_id)
            elif english_id:
                engine.setProperty('voice', english_id)

            print(f"🔊 Speaking: {text_to_speak}")
            engine.say(text_to_speak)
            engine.runAndWait()
            
            engine.stop()
            del engine
            
        except Exception as e:
            print(f"❌ TTS Error: {e}")
        
        speech_queue.task_done()
        time.sleep(0.1)


# --- ADD THIS FUNCTION (Place it near tts_worker) ---
def play_music_10s():
    """Plays song.mp3 for 10 seconds then stops"""
    try:
        pygame.mixer.music.load("song.mp3") # Make sure file exists!
        pygame.mixer.music.play()
        print("🎵 Playing Music...")
        
        # Wait 10 seconds while music plays
        time.sleep(17)
        
        # Fade out over 1 second and stop
        pygame.mixer.music.fadeout(1000)
        print("🎵 Music Stopped")
    except Exception as e:
        print(f"❌ Music Error: {e}")


# --- ADD THIS FUNCTION ---
def play_surprise_5s():
    """Plays Surprise.mp3 for 5 seconds then stops"""
    try:
        # Use the global mixer init
        pygame.mixer.music.load("Surprise.mp3") 
        pygame.mixer.music.play()
        print("🎵 Playing Surprise Sound...")
        
        # Adjust this time based on how long you want the sound to play
        time.sleep(5) 
        
        pygame.mixer.music.fadeout(1000)
        print("🎵 Surprise Stopped")
    except Exception as e:
        print(f"❌ Music Error: {e}")


def main():
    """Main function to run the face landmark detection"""
    # --- ADD THIS LINE HERE ---
    threading.Thread(target=tts_worker, daemon=True).start()
    
    while True:
        # Initialize serial connection to Pico
        # 2. CONNECT TO PICO (With Wait Loop)
        print("Searching for Raspberry Pi Pico on COM3...")
        sender = None
        
        while True:
            # Attempt connection
            sender = PicoSender(port='COM3', baudrate=115200)
            
            # Check if successful
            if sender.ser and sender.ser.is_open:
                # --- FIX: Prevent Freezing ---
                sender.ser.timeout = 0.05       # Don't wait > 0.05s for read
                sender.ser.write_timeout = 0.05 # Don't wait > 0.05s to send
                # -----------------------------
                print("\n✅ Successfully connected to Pico!")
                break
                
            # If failed, wait 2 seconds and try again
            # (You will see error messages from serial_sender.py until you plug it in - this is normal)
            print("⏳ Waiting for Pico... (Plug in USB)", end='\r')
            time.sleep(2)
        
        # Wait for START signal from Pico (can happen multiple times if user restarts)
        print("Waiting for START signal from Pico...")
        print("(On Pico: Enter password and press button)")
        
        # Reset tracking variables on each START
        expression_held_for_10s = set()  # Track which expressions have been held for 10s
        accumulated_time = {}  # Track accumulated time for each expression
        current_expression = 0
        expression_start_time = None
        last_expression = 0
        last_expression_change_time = None
        
        try:
            while True:
                if sender.ser.in_waiting > 0:
                    line = sender.ser.readline().decode('utf-8').strip()
                    print(f"Received: {line}")
                    # --- ADD THIS CHECK ---
                    if line.startswith("TTS:"):
                        msg = line[4:].strip()
                        if msg: speech_queue.put(msg)
                    # ----------------------
                    # --- ADD THIS NEW BLOCK ---
                    elif line.startswith("MUSIC:"):
                        # Run music in a separate thread so camera doesn't freeze
                        threading.Thread(target=play_music_10s, daemon=True).start()
                    # --------------------------
                    elif line.startswith("SURPRISE"):
                        threading.Thread(target=play_surprise_5s, daemon=True).start()
                    # --------------------------
                    elif line == "START":
                        print("START signal received! Initializing camera...")
                        # Reset all tracking variables when START is received
                        expression_held_for_10s.clear()
                        accumulated_time.clear()
                        print("Expression tracking reset for new attempt")
                        break
        except KeyboardInterrupt:
            print("\nInterrupted by user")
            sender.close()
            return
        except Exception as e:
            print(f"Error waiting for START signal: {e}")
            sender.close()
            continue
        
        # Initialize webcam
        print("Opening camera...")
        cap = cv2.VideoCapture(0)
        print(f"Camera opened: {cap.isOpened()}")
        
        if not cap.isOpened():
            print("Error: Could not open camera")
            sender.close() 
            continue
        
        print("Starting face landmark detection...")
        print("Press 'q' to quit")
        
        # Variables for tracking expression hold time (already reset when START received)
        if expression_start_time is None:
            expression_start_time = time.time()
            last_expression_change_time = time.time()
        
        # Initialize Face Mesh
        with mp_face_mesh.FaceMesh(
            max_num_faces=1,
            refine_landmarks=True,
            min_detection_confidence=0.5,
            min_tracking_confidence=0.5
        ) as face_mesh:
            
            while cap.isOpened():
                # Check for START signal to reset tracking (in case of retry)
                if sender.ser.in_waiting > 0:
                    try:
                        line = sender.ser.readline().decode('utf-8', errors='ignore').strip()
                        # --- ADD THIS CHECK ---
                        if line.startswith("TTS:"):
                            msg = line[4:].strip()
                            if msg: speech_queue.put(msg)
                        # ----------------------
                        # --- ADD THIS NEW BLOCK HERE ---
                        elif line.startswith("MUSIC:"):
                            # Run music in a separate thread so camera doesn't freeze
                            threading.Thread(target=play_music_10s, daemon=True).start()
                        # -------------------------------
                        elif line.startswith("SURPRISE"):
                            threading.Thread(target=play_surprise_5s, daemon=True).start()
                        # ------------------------------- 

                        elif line == "START":
                            print("\n=== RESTART DETECTED - Resetting expression tracking ===")
                            expression_held_for_10s.clear()
                            accumulated_time.clear()
                            current_expression = 0
                            expression_start_time = time.time()
                            last_expression = 0
                            last_expression_change_time = time.time()
                            print("Expression tracking reset for new attempt\n")
                    except Exception as e:
                        pass  # Ignore decode errors
                
                success, frame = cap.read()
                
                if not success:
                    print("Warning: Failed to read frame from camera")
                    continue
                
                # Flip the frame horizontally for a mirror view
                frame = cv2.flip(frame, 1)
                
                # Convert BGR to RGB for MediaPipe
                rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                
                # Process the frame
                results = face_mesh.process(rgb_frame)
                
                # Default text
                prominent_text = "No face detected"
                
                # If face landmarks are detected
                if results.multi_face_landmarks:
                    for face_landmarks in results.multi_face_landmarks:
                        # Draw face mesh tesselation (all 468 landmarks)
                        mp_drawing.draw_landmarks(
                            image=frame,
                            landmark_list=face_landmarks,
                            connections=mp_face_mesh.FACEMESH_TESSELATION,
                            landmark_drawing_spec=None,
                            connection_drawing_spec=mp_drawing_styles.get_default_face_mesh_tesselation_style()
                        )
                        
                        # Draw face contours (outline)
                        mp_drawing.draw_landmarks(
                            image=frame,
                            landmark_list=face_landmarks,
                            connections=mp_face_mesh.FACEMESH_CONTOURS,
                            landmark_drawing_spec=None,
                            connection_drawing_spec=mp_drawing_styles.get_default_face_mesh_contours_style()
                        )
                        
                        # Draw irises (eye details)
                        mp_drawing.draw_landmarks(
                            image=frame,
                            landmark_list=face_landmarks,
                            connections=mp_face_mesh.FACEMESH_IRISES,
                            landmark_drawing_spec=None,
                            connection_drawing_spec=mp_drawing_styles.get_default_face_mesh_iris_connections_style()
                        )
                        
                        # Detect expression
                        digit, expression_name = detect_expression(face_landmarks, frame.shape)
                        
                        current_time = time.time()
                        
                        # Check if expression changed
                        if digit != current_expression:
                            # Check if it's a brief interruption (less than 1 second)
                            time_since_last_change = current_time - last_expression_change_time
                            
                            if time_since_last_change < 1.0 and digit == last_expression:
                                # Brief interruption, continue counting from where we left off
                                current_expression = digit
                            else:
                                # Actual expression change - save accumulated time for previous expression
                                if current_expression == last_expression:
                                    time_to_add = current_time - expression_start_time
                                    if current_expression not in accumulated_time:
                                        accumulated_time[current_expression] = 0
                                    accumulated_time[current_expression] += time_to_add
                                
                                # Update to new expression
                                last_expression = current_expression
                                current_expression = digit
                                expression_start_time = current_time
                                last_expression_change_time = current_time
                                
                                # Send digit to Pico immediately on expression change
                                # --- FIX: Ignore errors if Pico is sleeping ---
                                try:
                                    sender.send_digit(digit)
                                except:
                                    pass
                        
                        # Calculate total time held (accumulated + current session)
                        if current_expression not in accumulated_time:
                            accumulated_time[current_expression] = 0
                        total_time_held = accumulated_time[current_expression] + (current_time - expression_start_time)
                        
                        # Check if expression has been held for 5 seconds total
                        if total_time_held >= 5.0 and digit not in expression_held_for_10s:
                            print(f"Good Job {digit}")
                            expression_held_for_10s.add(digit)
                            
                            # If digit 1 (smile), digit 6 (surprise), or digit 2 (right wink) was held for 10 seconds
                            if digit == 1 or digit == 6 or digit == 2:
                                try:
                                    sender.send_success(digit)
                                except:
                                    pass
                        
                        # Update display text
                        prominent_text = f"Digit {digit}: {expression_name}"
                
                # Display the expression text
                cv2.putText(
                    frame,
                    prominent_text,
                    (10, 40),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    1.0,
                    (0, 255, 0),
                    2,
                    cv2.LINE_AA
                )
                
                # Add instructions
                cv2.putText(
                    frame,
                    "Press 'q' to quit",
                    (10, frame.shape[0] - 20),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.6,
                    (255, 255, 255),
                    1,
                    cv2.LINE_AA
                )
                
                # Display the frame
                cv2.imshow('Facial Expression Detection (Digits 0-9)', frame)
                
                # Check for quit key
                if cv2.waitKey(5) & 0xFF == ord('q'):
                    break
        
        # Cleanup
        cap.release()
        cv2.destroyAllWindows()
        sender.close()
        print("Camera and serial connection closed successfully")


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\nProgram interrupted by user")
    except Exception as e:
        print(f"ERROR: {type(e).__name__}: {e}")
        import traceback
        traceback.print_exc()
    finally:
        input("Press Enter to exit...")

