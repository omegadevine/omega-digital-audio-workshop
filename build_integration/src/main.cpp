#include <iostream>
#include <SDL2/SDL.h>
#include "portaudio.h"

int main(int argc, char* argv[]) {
    std::cout << "==================================" << std::endl;
    std::cout << "  Omega Digital Audio Workshop" << std::endl;
    std::cout << "  Version 0.1.0 - Build Test" << std::endl;
    std::cout << "==================================" << std::endl;
    
    // Test PortAudio
    std::cout << "\nTesting PortAudio..." << std::endl;
    PaError err = Pa_Initialize();
    if (err == paNoError) {
        std::cout << "  PortAudio initialized successfully!" << std::endl;
        std::cout << "  Version: " << Pa_GetVersionText() << std::endl;
        Pa_Terminate();
    } else {
        std::cerr << "  PortAudio initialization failed: " << Pa_GetErrorText(err) << std::endl;
    }
    
    // Test SDL2
    std::cout << "\nTesting SDL2..." << std::endl;
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        SDL_version compiled;
        SDL_version linked;
        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);
        std::cout << "  SDL2 initialized successfully!" << std::endl;
        std::cout << "  Compiled against SDL version: " << (int)compiled.major << "." 
                  << (int)compiled.minor << "." << (int)compiled.patch << std::endl;
        std::cout << "  Linked SDL version: " << (int)linked.major << "."
                  << (int)linked.minor << "." << (int)linked.patch << std::endl;
        SDL_Quit();
    } else {
        std::cerr << "  SDL2 initialization failed: " << SDL_GetError() << std::endl;
    }
    
    std::cout << "\nAll dependencies verified successfully!" << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    
    return 0;
}
