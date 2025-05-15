cmake_minimum_required(VERSION 3.5)
project(opencv-mobile-test)

set(CMAKE_CXX_STANDARD 11)

# Đặt toolchain cross-compile
set(CMAKE_C_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-gcc")
set(CMAKE_CXX_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-g++")

# Tùy chỉnh flags cho kiến trúc RISC-V
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d")

# Include path và thư viện path của OpenCV
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/opencv-mobile-4.9.0-milkv-duo/include/opencv4)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/opencv-mobile-4.9.0-milkv-duo/lib)

add_executable(opencv-mobile-test main.cpp)

# Link thủ công các thư viện cần thiết
target_link_libraries(opencv-mobile-test
    opencv_core
    opencv_imgproc
    opencv_highgui
    opencv_objdetect
    opencv_face
    opencv_dnn         
    opencv_calib3d  
    opencv_features2d 
    opencv_flann
    opencv_core
    atomic
)

