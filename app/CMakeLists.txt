# CMakeLists.txt for app module
cmake_minimum_required(VERSION 3.22.1)

project(BitPerfectPlayer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Oboe package (provided via Gradle prefab)
find_package(oboe REQUIRED CONFIG)

# Create native library
add_library(
    audio-engine
    SHARED
    src/main/cpp/audio-engine.cpp
    src/main/cpp/jni-bridge.cpp
)

# Include directories
target_include_directories(
    audio-engine
    PRIVATE
    src/main/cpp
    ${OBOE_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(
    audio-engine
    PRIVATE
    oboe::oboe
    android
    log
    OpenSLES
)

# Compiler flags
target_compile_options(
    audio-engine
    PRIVATE
    -Wall
    -Wextra
    -Werror=format
    -Wno-unused-parameter
    -fexceptions
    -frtti
)

# Optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(audio-engine PRIVATE -O3)
else()
    target_compile_options(audio-engine PRIVATE -g)
endif()
