#include "DAWApplication.h"
#include "AudioFilePlayer.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace OmegaDAW;

void printHelp() {
    std::cout << "\n=== Audio File Playback Test ===\n";
    std::cout << "Commands:\n";
    std::cout << "  play   - Start/resume playback\n";
    std::cout << "  pause  - Pause playback\n";
    std::cout << "  stop   - Stop playback\n";
    std::cout << "  loop   - Toggle looping\n";
    std::cout << "  vol X  - Set volume (0.0 - 1.0)\n";
    std::cout << "  pos X  - Seek to position in seconds\n";
    std::cout << "  info   - Show file info and current position\n";
    std::cout << "  load   - Load a new audio file\n";
    std::cout << "  help   - Show this help\n";
    std::cout << "  quit   - Exit\n";
    std::cout << std::endl;
}

void displayStatus(const AudioFilePlayer& player, const AudioEngine& engine) {
    std::cout << "\n--- Status ---\n";
    if (player.isLoaded()) {
        std::cout << "File: " << player.getFilePath() << "\n";
        std::cout << "Duration: " << player.getDuration() << " seconds\n";
        std::cout << "Position: " << player.getPosition() << " / " << player.getDuration() << " seconds\n";
        std::cout << "State: " << (player.isPlaying() ? "Playing" : (player.isPaused() ? "Paused" : "Stopped")) << "\n";
        std::cout << "Loop: " << (player.isLooping() ? "ON" : "OFF") << "\n";
        std::cout << "Volume: " << (player.getVolume() * 100.0f) << "%\n";
        std::cout << "Sample Rate: " << player.getFileSampleRate() << " Hz (Engine: " << engine.getSampleRate() << " Hz)\n";
        std::cout << "Channels: " << player.getFileChannels() << "\n";
        
        // Show meters
        std::cout << "Meters: L=" << (engine.getPeakLevel(0) * 100.0f) << "% ";
        if (engine.getNumChannels() > 1) {
            std::cout << "R=" << (engine.getPeakLevel(1) * 100.0f) << "%";
        }
        std::cout << "\n";
    } else {
        std::cout << "No file loaded\n";
    }
    std::cout << "CPU Load: " << (engine.getCPULoad() * 100.0f) << "%\n";
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "=== Omega DAW - Audio File Playback Test ===\n" << std::endl;
    
    // Initialize audio engine
    AudioEngine audioEngine;
    if (!audioEngine.initialize(48000, 256, 2)) {
        std::cerr << "Failed to initialize audio engine" << std::endl;
        return 1;
    }
    
    // Create audio file player
    auto player = std::make_shared<AudioFilePlayer>();
    audioEngine.addProcessor(player);
    
    // Start audio engine
    audioEngine.startPlayback();
    
    // Load file from command line if provided
    std::string initialFile;
    if (argc > 1) {
        initialFile = argv[1];
        std::cout << "Loading file: " << initialFile << std::endl;
        if (player->loadFile(initialFile)) {
            std::cout << "File loaded successfully!" << std::endl;
            displayStatus(*player, audioEngine);
        } else {
            std::cerr << "Failed to load file" << std::endl;
        }
    } else {
        std::cout << "Usage: " << argv[0] << " <audio_file.wav>" << std::endl;
        std::cout << "Or use 'load' command to load a file interactively\n" << std::endl;
    }
    
    printHelp();
    
    // Interactive command loop
    std::string command;
    bool running = true;
    
    while (running) {
        std::cout << "> ";
        std::cin >> command;
        
        if (command == "quit" || command == "exit" || command == "q") {
            running = false;
        }
        else if (command == "play" || command == "p") {
            player->play();
        }
        else if (command == "pause") {
            player->pause();
        }
        else if (command == "stop" || command == "s") {
            player->stop();
        }
        else if (command == "loop" || command == "l") {
            player->setLoop(!player->isLooping());
            std::cout << "Loop: " << (player->isLooping() ? "ON" : "OFF") << std::endl;
        }
        else if (command == "vol" || command == "v") {
            float volume;
            std::cin >> volume;
            player->setVolume(volume);
            std::cout << "Volume set to " << (volume * 100.0f) << "%" << std::endl;
        }
        else if (command == "pos") {
            double position;
            std::cin >> position;
            player->setPosition(position);
            std::cout << "Position set to " << position << " seconds" << std::endl;
        }
        else if (command == "info" || command == "i") {
            displayStatus(*player, audioEngine);
        }
        else if (command == "load") {
            std::string filepath;
            std::cout << "Enter audio file path: ";
            std::cin >> filepath;
            
            if (player->loadFile(filepath)) {
                std::cout << "File loaded successfully!" << std::endl;
                displayStatus(*player, audioEngine);
            } else {
                std::cerr << "Failed to load file" << std::endl;
            }
        }
        else if (command == "help" || command == "h" || command == "?") {
            printHelp();
        }
        else {
            std::cout << "Unknown command. Type 'help' for list of commands." << std::endl;
        }
    }
    
    // Clean up
    std::cout << "\nShutting down..." << std::endl;
    player->stop();
    audioEngine.stopPlayback();
    audioEngine.shutdown();
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}
