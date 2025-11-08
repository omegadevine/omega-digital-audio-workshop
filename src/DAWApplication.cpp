#include "DAWApplication.h"
#include <iostream>

namespace OmegaDAW {

DAWApplication::DAWApplication() 
    : running(false), initialized(false) {
}

DAWApplication::~DAWApplication() {
    shutdown();
}

bool DAWApplication::initialize() {
    std::cout << "Initializing Omega DAW Application..." << std::endl;
    
    try {
        // Create all components
        audioEngine = std::make_unique<AudioEngine>();
        midiDevice = std::make_unique<MIDIDevice>();
        midiSequencer = std::make_unique<MIDISequencer>();
        pluginHost = std::make_unique<PluginHost>();
        mixer = std::make_unique<Mixer>();
        router = std::make_unique<Router>();
        sequencer = std::make_unique<Sequencer>();
        arrangement = std::make_unique<Arrangement>();
        transport = std::make_unique<Transport>();
        project = std::make_unique<Project>();
        fileIO = std::make_unique<FileIO>();
        uiWindow = std::make_unique<UIWindow>();
        
        // Initialize components
        if (!audioEngine->initialize(44100, 512, 2)) {
            std::cerr << "Failed to initialize audio engine" << std::endl;
            return false;
        }
        
        if (!midiDevice->initialize()) {
            std::cerr << "Warning: Failed to initialize MIDI device" << std::endl;
        }
        
        midiSequencer->initialize();
        pluginHost->initialize();
        mixer->initialize(audioEngine->getSampleRate(), audioEngine->getBufferSize());
        router->initialize();
        sequencer->initialize();
        arrangement->initialize();
        transport->initialize();
        
        if (!uiWindow->initialize("Omega DAW", 1280, 800)) {
            std::cerr << "Failed to initialize UI window" << std::endl;
            return false;
        }
        
        // Connect components
        connectComponents();
        
        initialized = true;
        running = true;
        
        std::cout << "Omega DAW initialized successfully!" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception during initialization: " << e.what() << std::endl;
        return false;
    }
}

void DAWApplication::connectComponents() {
    // Connect MIDI device to sequencer
    midiDevice->setMessageCallback([this](const MIDIMessage& msg) {
        midiSequencer->processMIDIMessage(msg);
    });
    
    // Connect transport to sequencer and arrangement
    transport->setPlayCallback([this]() {
        sequencer->start();
        arrangement->start();
    });
    
    transport->setStopCallback([this]() {
        sequencer->stop();
        arrangement->stop();
    });
    
    // Connect sequencer to mixer through router
    sequencer->setOutputCallback([this](const AudioBuffer& buffer) {
        router->routeAudio(buffer, mixer.get());
    });
    
    // Connect mixer to audio engine
    mixer->setOutputCallback([this](const AudioBuffer& buffer) {
        audioEngine->processBuffer(buffer);
    });
    
    // Connect plugin host to mixer
    pluginHost->setMixer(mixer.get());
    
    std::cout << "Component connections established" << std::endl;
}

void DAWApplication::shutdown() {
    if (!initialized) return;
    
    std::cout << "Shutting down Omega DAW..." << std::endl;
    
    running = false;
    
    // Stop playback
    stop();
    
    // Shutdown components in reverse order
    if (uiWindow) uiWindow->shutdown();
    if (transport) transport->shutdown();
    if (arrangement) arrangement->shutdown();
    if (sequencer) sequencer->shutdown();
    if (router) router->shutdown();
    if (mixer) mixer->shutdown();
    if (pluginHost) pluginHost->shutdown();
    if (midiSequencer) midiSequencer->shutdown();
    if (midiDevice) midiDevice->shutdown();
    if (audioEngine) audioEngine->shutdown();
    
    initialized = false;
    
    std::cout << "Omega DAW shutdown complete" << std::endl;
}

bool DAWApplication::run() {
    if (!initialized) {
        std::cerr << "Cannot run - application not initialized" << std::endl;
        return false;
    }
    
    while (running && uiWindow->isOpen()) {
        // Process events
        processEvents();
        
        // Process audio
        processAudio();
        
        // Update UI
        updateUI();
    }
    
    return true;
}

void DAWApplication::processAudio() {
    if (!transport->isPlaying()) return;
    
    // Get current playback position
    double position = transport->getPosition();
    
    // Process MIDI sequencer
    auto midiEvents = midiSequencer->getEventsAtPosition(position);
    for (const auto& event : midiEvents) {
        pluginHost->processMIDI(event);
    }
    
    // Process arrangement
    auto audioBuffer = arrangement->renderAtPosition(position);
    
    // Route through mixer
    mixer->process(audioBuffer);
    
    // Advance transport
    transport->advance();
}

void DAWApplication::processEvents() {
    uiWindow->processEvents();
    midiDevice->processEvents();
}

void DAWApplication::updateUI() {
    uiWindow->beginFrame();
    
    // Update transport UI
    uiWindow->drawTransport(transport.get());
    
    // Update mixer UI
    uiWindow->drawMixer(mixer.get());
    
    // Update timeline/arrangement UI
    uiWindow->drawTimeline(arrangement.get(), transport->getPosition());
    
    uiWindow->endFrame();
}

bool DAWApplication::newProject(const std::string& projectName) {
    project->clear();
    project->setName(projectName);
    
    // Initialize default project structure
    arrangement->clear();
    mixer->reset();
    transport->reset();
    
    std::cout << "New project created: " << projectName << std::endl;
    return true;
}

bool DAWApplication::loadProject(const std::string& filepath) {
    if (!fileIO->loadProject(filepath, project.get())) {
        std::cerr << "Failed to load project: " << filepath << std::endl;
        return false;
    }
    
    // Apply project data to components
    arrangement->loadFromProject(project.get());
    mixer->loadFromProject(project.get());
    
    std::cout << "Project loaded: " << filepath << std::endl;
    return true;
}

bool DAWApplication::saveProject(const std::string& filepath) {
    // Gather current state into project
    project->setArrangementData(arrangement->serialize());
    project->setMixerData(mixer->serialize());
    
    if (!fileIO->saveProject(filepath, project.get())) {
        std::cerr << "Failed to save project: " << filepath << std::endl;
        return false;
    }
    
    std::cout << "Project saved: " << filepath << std::endl;
    return true;
}

void DAWApplication::play() {
    transport->play();
}

void DAWApplication::stop() {
    transport->stop();
}

void DAWApplication::pause() {
    transport->pause();
}

void DAWApplication::record() {
    transport->record();
}

bool DAWApplication::isPlaying() const {
    return transport && transport->isPlaying();
}

bool DAWApplication::isRecording() const {
    return transport && transport->isRecording();
}

} // namespace OmegaDAW
