cmake_minimum_required(VERSION 3.15)
project(OmegaDAW_IntegrationTest VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find PortAudio (should be in system paths or CMAKE_PREFIX_PATH)
find_library(PORTAUDIO_LIBRARY NAMES portaudio portaudio_x64 PATHS "C:/Program Files/portaudio" "C:/portaudio" ENV PATH)
find_path(PORTAUDIO_INCLUDE_DIR portaudio.h PATHS "C:/Program Files/portaudio/include" "C:/portaudio/include" ENV PATH)

if(NOT PORTAUDIO_LIBRARY OR NOT PORTAUDIO_INCLUDE_DIR)
    message(STATUS "PortAudio not found in system, using manual library")
    # Try to find it in Windows common locations
    set(PORTAUDIO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/portaudio/include")
    set(PORTAUDIO_LIBRARY "${CMAKE_SOURCE_DIR}/external/portaudio/lib/portaudio_x64.lib")
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${PORTAUDIO_INCLUDE_DIR}
)

set(SOURCES
    src/AudioEngine.cpp
    src/AudioBuffer.cpp
    src/MIDIMessage.cpp
    src/MIDISequencer.cpp
    src/MIDISynthesizer.cpp
    src/main_integration_test.cpp
)

add_executable(OmegaDAW_IntegrationTest ${SOURCES})

target_link_libraries(OmegaDAW_IntegrationTest PRIVATE ${PORTAUDIO_LIBRARY})

if(WIN32)
    target_link_libraries(OmegaDAW_IntegrationTest PRIVATE winmm)
endif()

set_target_properties(OmegaDAW_IntegrationTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
