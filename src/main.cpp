#include "AudioEngine.h"
#include <iostream>

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << " Omega Digital Audio Workshop" << std::endl;
    std::cout << " Version 0.1.0" << std::endl;
    std::cout << "==================================" << std::endl;
    
    omega::AudioEngine engine;
    
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize audio engine" << std::endl;
        return 1;
    }
    
    std::cout << "\nAudio engine ready!" << std::endl;
    std::cout << "Sample Rate: " << engine.getSampleRate() << " Hz" << std::endl;
    std::cout << "Buffer Size: " << engine.getBufferSize() << " samples" << std::endl;
    
    // Basic test
    engine.startPlayback();
    std::cout << "Is playing: " << (engine.isPlaying() ? "Yes" : "No") << std::endl;
    
    engine.stopPlayback();
    std::cout << "Is playing: " << (engine.isPlaying() ? "Yes" : "No") << std::endl;
    
    engine.shutdown();
    
    std::cout << "\nPress Enter to exit..." << std::endl;
    std::cin.get();
    
    return 0;
}
