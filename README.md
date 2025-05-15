# Driver Drowsiness Detection using Milk-V Duo 256M and CAM-GC2083 (Linux - C++)

This project implements a real-time driver drowsiness detection system using the Milk-V Duo 256M development board and the CAM-GC2083 camera module. The system captures live facial images, analyzes eye closure and head position patterns, and identifies early signs of drowsiness using computer vision techniques.: 

The goal is to improve road safety by alerting the driver when signs of fatigue or microsleep are detected.

---

## Key Features

- Real-time video capture using GC2083 sensor
- Lightweight edge processing with Milk-V Duo 256M (RISC-V)
- Drowsiness detection based on eye aspect ratio (EAR) and head orientation
- Visual and/or audible alerts for drowsy states
- Optimized for embedded performance and low power consumption

---

## Hardware Used
- Duo256M: https://milkv.io/docs/duo/getting-started/duo256m
- CAMGC-2083: https://milkv.io/docs/duo/camera/gc2083

---
## Code Requirement 
- You should read documents and instruction from the official page of MilkV: https://milkv.io/docs/duo/getting-started/boot
- Besides you need :
    - lbfmodel
    - haarcascade_frontalface_default.xml
    - opencv-mobile (A lightweight OpenCV library, but in this project I customized it to run my code

 ---
## Methodology 
![image](https://github.com/user-attachments/assets/24b1ab25-ec41-4766-9a4f-5568494e374d)

## Algorithm Flowchart


