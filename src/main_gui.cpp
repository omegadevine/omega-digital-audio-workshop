#include "DAWApplication.h"
#include "DAWGUI.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace OmegaDAW;

int main(int argc, char* argv[]) {
    std::cout << "=== Omega Digital Audio Workshop ===" << std::endl;
    std::cout << "Initializing..." << std::endl;
    
    // Create DAW application
    auto daw = std::make_unique<DAWApplication>();
    
    // Initialize audio engine
    if (!daw->initialize(44100, 512)) {
        std::cerr << "Failed to initialize DAW application" << std::endl;
        return 1;
    }
    
    std::cout << "DAW initialized successfully" << std::endl;
    
    // Create and initialize GUI
    auto gui = std::make_unique<DAWGUI>(daw.get());
    if (!gui->initialize(1280, 720)) {
        std::cerr << "Failed to initialize GUI" << std::endl;
        return 1;
    }
    
    std::cout << "GUI initialized successfully" << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  SPACE - Play/Stop" << std::endl;
    std::cout << "  R     - Record" << std::endl;
    std::cout << "  ESC   - Quit" << std::endl;
    std::cout << "\nGUI is ready!" << std::endl;
    
    // Main loop
    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    double fpsTimer = 0.0;
    
    while (!gui->shouldQuit()) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        
        // Process events
        gui->processEvents();
        
        // Render
        gui->render();
        
        // Calculate frame time
        auto frameEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> frameDuration = frameEnd - frameStart;
        
        // FPS counter
        frameCount++;
        fpsTimer += frameDuration.count();
        if (fpsTimer >= 1.0) {
            std::cout << "FPS: " << frameCount << " | Audio CPU: " 
                      << (int)(daw->getAudioEngine()->getCPULoad() * 100) << "%" << std::endl;
            frameCount = 0;
            fpsTimer = 0.0;
        }
        
        // Target 60 FPS (optional frame limiting)
        const double targetFrameTime = 1.0 / 60.0;
        if (frameDuration.count() < targetFrameTime) {
            std::this_thread::sleep_for(
                std::chrono::duration<double>(targetFrameTime - frameDuration.count())
            );
        }
    }
    
    std::cout << "\nShutting down..." << std::endl;
    gui->shutdown();
    daw->shutdown();
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}
