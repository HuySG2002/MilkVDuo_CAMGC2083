import cv2
import dlib
import time
import numpy as np
from imutils import face_utils

# Load the face detector and facial landmark predictor
detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor("/home/hyuuuan/Downloads/shape_predictor_68_face_landmarks.dat")  # Download required

# Initialize webcam
cap = cv2.VideoCapture("/home/hyuuuan/Documents/Drowsiness/drowsiness_detection/src/sample_videos/driver_day.mp4")

# Initialize start times for different conditions
eye_closed_start_time = None
mouth_open_start_time = None
head_off_center_start_time = None

# Threshold settings (in seconds or pixels)
EYE_CLOSED_THRESHOLD = 1
MOUTH_OPEN_THRESHOLD = 1
HEAD_OFF_CENTER_THRESHOLD = 2

# Helper function: Calculates Euclidean distance between two points
def distance(p1, p2):
    return np.linalg.norm(np.array(p1) - np.array(p2))

# Function: detect_drowsiness
# Arguments:
#   - landmarks: array of facial landmark points
#   - frame_width: width of the video frame
# Global Variables Used:
#   - eye_closed_start_time, mouth_open_start_time, head_off_center_start_time
# Functionality:
#   - Detects if the person is drowsy based on eye closure
#   - Detects yawning based on mouth openness
#   - Detects if the head is off-center (lack of attention)
# Summary:
#   This function compares distances between eye/mouth landmarks
#   and uses timers to determine if abnormal behavior persists
#   for more than a threshold time.
def detect_drowsiness(landmarks, frame_width):
    global eye_closed_start_time, mouth_open_start_time, head_off_center_start_time

    # Get necessary landmark points
    left_eye_top = landmarks[37]
    left_eye_bottom = landmarks[41]
    right_eye_top = landmarks[43]
    right_eye_bottom = landmarks[47]
    mouth_top = landmarks[62]
    mouth_bottom = landmarks[66]
    center_points = [landmarks[30], landmarks[33], landmarks[36], landmarks[45], landmarks[8]]
    center_x = np.mean([pt[0] for pt in center_points])

    # Calculate eye openness and mouth openness
    eye_open_avg = (distance(left_eye_top, left_eye_bottom) + distance(right_eye_top, right_eye_bottom)) / 2
    mouth_open = distance(mouth_top, mouth_bottom)

    # Detect eye closure (drowsiness)
    if eye_open_avg < 11:
        if eye_closed_start_time is None:
            eye_closed_start_time = time.time()
        elif time.time() - eye_closed_start_time > EYE_CLOSED_THRESHOLD:
            return "Drowsiness"
    else:
        eye_closed_start_time = None

    # Detect yawning
    if mouth_open > 12:
        if mouth_open_start_time is None:
            mouth_open_start_time = time.time()
        elif time.time() - mouth_open_start_time > MOUTH_OPEN_THRESHOLD:
            return "Yawning"
    else:
        mouth_open_start_time = None

    # Detect loss of focus (head off center)
    offset = abs(center_x - frame_width // 2)
    if offset > 40:
        if head_off_center_start_time is None:
            head_off_center_start_time = time.time()
        elif time.time() - head_off_center_start_time > HEAD_OFF_CENTER_THRESHOLD:
            return "Unfocused"
    else:
        head_off_center_start_time = None

    return "Normal"

# Main loop for video processing
while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = cv2.flip(frame, 1)  # Flip horizontally for natural view
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = detector(gray)

    if not faces:
        cv2.putText(frame, "No face detected!", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
    else:
        for face in faces:
            shape = predictor(gray, face)
            landmarks = face_utils.shape_to_np(shape)

            # Draw facial landmarks
            for (i, (x, y)) in enumerate(landmarks):
                cv2.circle(frame, (x, y), 1, (0, 255, 0), -1)
                cv2.putText(frame, str(i), (x + 2, y - 2), cv2.FONT_HERSHEY_SIMPLEX, 0.3, (0, 255, 255), 1)

            # Get status from drowsiness detection
            status = detect_drowsiness(landmarks, frame.shape[1])

            # Display detection status
            cv2.putText(frame, f'Status: {status}', (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.9,
                        (0, 0, 255) if status != "Normal" else (0, 255, 0), 2)

    cv2.imshow("Drowsiness Detection", frame)
    if cv2.waitKey(1) == 27:  # Press ESC to exit
        break

cap.release()
cv2.destroyAllWindows()
