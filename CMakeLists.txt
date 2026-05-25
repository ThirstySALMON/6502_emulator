cmake_minimum_required(VERSION 4.0)
project(6502_Emulator)

set(CMAKE_CXX_STANDARD 17)

add_executable(6502_Emulator src/main.cpp
        src/CPU.cpp
        src/CPU.h
        src/Memory.cpp
        src/Memory.h
        src/incl.h)
