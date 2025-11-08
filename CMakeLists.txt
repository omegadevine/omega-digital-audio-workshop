cmake_minimum_required(VERSION 3.15)
project(OmegaDAW VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find PortAudio
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(PORTAUDIO portaudio-2.0)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_TESTS "Build test suite" OFF)
option(BUILD_EXAMPLES "Build example applications" OFF)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${PORTAUDIO_INCLUDE_DIRS}
)

# Source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.c"
)

file(GLOB_RECURSE HEADERS 
    "include/*.h"
    "include/*.hpp"
)

# Main executable
add_executable(OmegaDAW 
    ${SOURCES}
    ${HEADERS}
)

# Link PortAudio
if(PORTAUDIO_FOUND)
    target_link_libraries(OmegaDAW ${PORTAUDIO_LIBRARIES})
    target_include_directories(OmegaDAW PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
endif()

# Platform-specific settings
if(WIN32)
    target_compile_definitions(OmegaDAW PRIVATE OMEGA_PLATFORM_WINDOWS)
    # Link Windows audio libraries
    target_link_libraries(OmegaDAW 
        winmm
        dsound
        ole32
        oleaut32
    )
    # If PortAudio not found via pkg-config, try manual link
    if(NOT PORTAUDIO_FOUND)
        target_link_libraries(OmegaDAW portaudio)
    endif()
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(OmegaDAW PRIVATE OMEGA_PLATFORM_LINUX)
    target_link_libraries(OmegaDAW 
        asound
        pthread
    )
elseif(APPLE)
    target_compile_definitions(OmegaDAW PRIVATE OMEGA_PLATFORM_MAC)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(COREMIDI_LIBRARY CoreMIDI)
    target_link_libraries(OmegaDAW 
        ${COREAUDIO_LIBRARY}
        ${COREMIDI_LIBRARY}
    )
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(OmegaDAW PRIVATE /W4)
else()
    target_compile_options(OmegaDAW PRIVATE -Wall -Wextra -pedantic)
endif()

# Installation rules
install(TARGETS OmegaDAW DESTINATION bin)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

message(STATUS "Omega DAW Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
