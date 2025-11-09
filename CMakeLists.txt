cmake_minimum_required(VERSION 3.15)
project(OmegaDAW VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find SDL2
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

# Find PortAudio
find_package(portaudio CONFIG REQUIRED)

# Find nlohmann_json
find_package(nlohmann_json CONFIG REQUIRED)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Source files for main GUI application
set(SOURCES 
    src/AdvancedEffects.cpp
    src/Arrangement.cpp
    src/AudioBuffer.cpp
    src/AudioEngine.cpp
    src/AudioFilePlayer.cpp
    src/AudioProcessing.cpp
    src/BuiltInPlugins.cpp
    src/Clip.cpp
    src/DAWApplication.cpp
    src/DAWGUI.cpp
    src/Effects.cpp
    src/FileIO.cpp
    src/Filter.cpp
    src/MIDIDevice.cpp
    src/MIDIFile.cpp
    src/MIDIMessage.cpp
    src/MIDISequencer.cpp
    src/MIDISynthesizer.cpp
    src/Mixer.cpp
    src/MixerChannel.cpp
    src/Oscillator.cpp
    src/Plugin.cpp
    src/PluginHost.cpp
    src/Project.cpp
    src/Router.cpp
    src/Sequencer.cpp
    src/Track.cpp
    src/Transport.cpp
    src/UIControls.cpp
    src/UIMixer.cpp
    src/UITimeline.cpp
    src/UITransport.cpp
    src/UIWindow.cpp
    src/main_full.cpp
)

# Main executable
add_executable(OmegaDAW ${SOURCES})

# Link libraries
target_link_libraries(OmegaDAW 
    PRIVATE 
    portaudio
    nlohmann_json::nlohmann_json
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(OmegaDAW PRIVATE OMEGA_PLATFORM_WINDOWS)
    target_link_libraries(OmegaDAW 
        PRIVATE
        winmm
        dsound
        ole32
        oleaut32
    )
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(OmegaDAW PRIVATE OMEGA_PLATFORM_LINUX)
    target_link_libraries(OmegaDAW 
        PRIVATE
        asound
        pthread
    )
elseif(APPLE)
    target_compile_definitions(OmegaDAW PRIVATE OMEGA_PLATFORM_MAC)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(COREMIDI_LIBRARY CoreMIDI)
    target_link_libraries(OmegaDAW 
        PRIVATE
        ${COREAUDIO_LIBRARY}
        ${COREMIDI_LIBRARY}
    )
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(OmegaDAW PRIVATE /W3)
else()
    target_compile_options(OmegaDAW PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

# Installation rules
install(TARGETS OmegaDAW DESTINATION bin)

message(STATUS "Omega DAW Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
