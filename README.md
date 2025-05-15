# Driver Drowsiness Detection using Milk-V Duo 256M and CAM-GC2083 (Linux - C++)

This project implements a real-time driver drowsiness detection system using the Milk-V Duo 256M development board and the CAM-GC2083 camera module. The system captures live facial images, analyzes eye closure and head position patterns, and identifies early signs of drowsiness using computer vision techniques.: 

The goal is to improve road safety by alerting the driver when signs of fatigue or microsleep are detected.

---
## License

This project is a modified version of https://github.com/kurshakuz/drowsiness_detection.git, originally licensed under the MIT License.

Modifications were made by [HuySG2002] to adapt the software for [driver drowsiness detection on Milk-V Duo 256M].


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
- Review the official Milk-V Duo documentation: https://milkv.io/docs/duo/getting-started/boot
- Besides you need :
    - lbfmodel - for facial landmark detection
    - haarcascade_frontalface_default.xml - for face detection
    - opencv-mobile (a lightweight OpenCV library (customized for this project): https://drive.google.com/drive/folders/168lecYnUgDzVX6850an984UIn6MJvEtl?usp=drive_link

 ---
## Methodology 
![image](https://github.com/user-attachments/assets/24b1ab25-ec41-4766-9a4f-5568494e374d)

## Algorithm Flowchart
![image](https://github.com/user-attachments/assets/4eaea262-695a-488b-acb9-fbfbf1b09551)

---
## Execution
  
To run the code in MilkVDuo256M with CAM-GC2083: 
- In PC: 
  * If you want to run it without board, I recommend you run:

    g++ full_drowsiness_estimation.cpp -o drowsiness `pkg-config --cflags --libs opencv4` -std=c++11
    
  * You need to create a folder lib in board first (mkdir lib) to restore *so files.
    - scp -O /home/user-name/opencv-mobile-4.9.0-milkv-duo/lib/*.so* root@192.168.42.1:/root/lib/: I had to cross-compile original opencv and opencv-contrib, so you need to move *so file into board        to use. 
    - cd build && scp -O opencv-mobile-test root@192.168.42.1:/root/
    
- In Board MilkVDuo256M:
      - chmod +x opencv-mobile-test && LD_LIBRARY_PATH=/root/lib ./opencv-mobile-test

## Result: 
https://drive.google.com/drive/folders/168lecYnUgDzVX6850an984UIn6MJvEtl?usp=drive_link
![image](https://github.com/user-attachments/assets/f38315b2-7700-4c17-b59d-4b5ea6bd7c83)


--- 

