cmake_minimum_required(VERSION 3.15)
project(emu6502)
set(CMAKE_CXX_STANDARD 17)

# --- Main emulator library ---
# Only .cpp files, no main.cpp, no headers
add_library(6502_lib
        src/CPU.cpp
        src/Memory.cpp
)
target_include_directories(6502_lib PUBLIC src)

# --- Main executable ---
add_executable(6502_Emulator src/main.cpp)
target_link_libraries(6502_Emulator 6502_lib)

# --- Google Test ---
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# --- Test executable ---
add_executable(emu_tests
        tests/test_lda.cpp
)
target_link_libraries(emu_tests 6502_lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(emu_tests)