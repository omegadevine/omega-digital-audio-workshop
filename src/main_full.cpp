#include "DAWApplication.h"
#include "DAWGUI.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "==================================" << std::endl;
    std::cout << "  Omega Digital Audio Workshop" << std::endl;
    std::cout << "  Version 0.1.0" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << std::endl;
    
    // Create and initialize the DAW application
    OmegaDAW::DAWApplication daw;
    
    if (!daw.initialize()) {
        std::cerr << "Failed to initialize DAW application" << std::endl;
        return 1;
    }
    
    // Create a new project
    daw.newProject("Untitled Project");
    
    // Initialize GUI
    OmegaDAW::DAWGUI gui(&daw);
    if (!gui.initialize(1280, 800)) {
        std::cerr << "Failed to initialize GUI" << std::endl;
        return 1;
    }
    
    std::cout << std::endl;
    std::cout << "DAW is ready!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  SPACE - Play/Stop" << std::endl;
    std::cout << "  R - Record" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << std::endl;
    
    // Main GUI loop
    while (!gui.shouldQuit()) {
        gui.processEvents();
        daw.processAudio();
        gui.render();
    }
    
    // Cleanup
    gui.shutdown();
    // daw cleanup handled by destructor
    
    return 0;
}
