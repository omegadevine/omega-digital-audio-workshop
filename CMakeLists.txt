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

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Simple GUI executable (standalone, no DAW dependencies)
add_executable(OmegaDAW_SimpleGUI
    src/main_simple_gui.cpp
)

target_link_libraries(OmegaDAW_SimpleGUI 
    PRIVATE 
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
)

# Source files for DAW library
set(DAW_SOURCES
    src/AudioEngine.cpp
    src/AudioBuffer.cpp
    src/MIDIMessage.cpp
    src/MIDIDevice.cpp
    src/MIDIFile.cpp
    src/MIDISequencer.cpp
    src/Oscillator.cpp
    src/Filter.cpp
    src/Effects.cpp
    src/Plugin.cpp
    src/PluginHost.cpp
    src/BuiltInPlugins.cpp
    src/Sequencer.cpp
    src/Track.cpp
    src/Clip.cpp
    src/Arrangement.cpp
    src/Transport.cpp
    src/Mixer.cpp
    src/MixerChannel.cpp
    src/Router.cpp
    src/FileIO.cpp
    src/Project.cpp
    src/DAWApplication.cpp
    src/UIWindow.cpp
    src/UIControls.cpp
    src/UITransport.cpp
    src/UIMixer.cpp
    src/UITimeline.cpp
    src/DAWGUI.cpp
)

# Main GUI executable
add_executable(OmegaDAW_GUI
    src/main_gui.cpp
    ${DAW_SOURCES}
)

# Link libraries
target_link_libraries(OmegaDAW_GUI 
    PRIVATE 
    portaudio
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
)

# Simple test executable
add_executable(OmegaDAW_Test
    src/main.cpp
)

target_link_libraries(OmegaDAW_Test 
    PRIVATE 
    portaudio
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(OmegaDAW_GUI PRIVATE OMEGA_PLATFORM_WINDOWS)
    target_compile_definitions(OmegaDAW_Test PRIVATE OMEGA_PLATFORM_WINDOWS)
    
    target_link_libraries(OmegaDAW_GUI 
        PRIVATE
        winmm
        dsound
        ole32
        oleaut32
    )
    
    target_link_libraries(OmegaDAW_Test 
        PRIVATE
        winmm
        dsound
        ole32
        oleaut32
    )
endif()

message(STATUS "Omega DAW Test Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
