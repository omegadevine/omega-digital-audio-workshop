#include "DAWApplication.h"
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
    
    std::cout << std::endl;
    std::cout << "DAW is ready!" << std::endl;
    std::cout << "Press ESC to exit..." << std::endl;
    std::cout << std::endl;
    
    // Run the main application loop
    daw.run();
    
    // Cleanup is handled by destructor
    
    return 0;
}
