# riscv-toolchain.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

set(TOOLCHAIN_DIR /home/hyuuuan/Documents/Drowsiness_Final/host-tools/gcc/riscv64-linux-musl-x86_64)

set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/riscv64-unknown-linux-musl-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/riscv64-unknown-linux-musl-g++)

set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR}/riscv64-unknown-linux-musl)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
