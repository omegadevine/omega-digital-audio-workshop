#include "DAWApplication.h"
#include "MIDIDevice.h"
#include "MIDISynthesizer.h"
#include <iostream>

namespace OmegaDAW {

DAWApplication::DAWApplication() 
    : running(false), initialized(false), midiSynth(nullptr) {
}

DAWApplication::~DAWApplication() {
    shutdown();
}

bool DAWApplication::initialize() {
    return initialize(44100, 512);
}

bool DAWApplication::initialize(int sampleRate, int bufferSize) {
    std::cout << "Initializing Omega DAW Application..." << std::endl;
    
    try {
        // Create all components
        audioEngine = std::make_unique<AudioEngine>();
        midiSequencer = std::make_unique<MIDISequencer>();
        pluginHost = std::make_unique<PluginHost>();
        mixer = std::make_unique<Mixer>();
        router = std::make_unique<Router>();
        arrangement = std::make_unique<Arrangement>();
        transport = std::make_unique<Transport>();
        project = std::make_unique<Project>();
        fileIO = &FileManager::getInstance();
        
        // Sequencer needs AudioEngine reference
        sequencer = std::make_unique<Sequencer>(*audioEngine);
        
        // UIWindow needs title and dimensions
        uiWindow = std::make_unique<UIWindow>("Omega DAW", 1280, 720);
        
        // Initialize components
        if (!audioEngine->initialize(sampleRate, bufferSize, 2)) {
            std::cerr << "Failed to initialize audio engine" << std::endl;
            return false;
        }
        
        // if (!midiDevice->initialize()) {
        //     std::cerr << "Warning: Failed to initialize MIDI device" << std::endl;
        // }
        
        // midiSequencer->initialize();
        // pluginHost->initialize();
        mixer->initialize(audioEngine->getSampleRate(), audioEngine->getBufferSize());
        // router->initialize();
        // sequencer->initialize();
        arrangement->initialize();
        transport->initialize();
        
        if (!uiWindow->initialize("Omega DAW", 1280, 800)) {
            std::cerr << "Failed to initialize UI window" << std::endl;
            return false;
        }
        
        // Create and add MIDI synthesizer
        midiSynth = std::make_shared<MIDISynthesizer>(16);
        midiSynth->prepare(sampleRate, bufferSize);
        audioEngine->addProcessor(midiSynth);
        
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
    // midiDevice->setMessageCallback([this](const MIDIMessage& msg) {
    //     midiSequencer->processMIDIMessage(msg);
    // });
    
    // Connect transport to sequencer and arrangement
    transport->setPlayCallback([this]() {
        // sequencer->start();
        arrangement->start();
    });
    
    transport->setStopCallback([this]() {
        // sequencer->stop();
        arrangement->stop();
    });
    
    // Connect sequencer to mixer through router
    // sequencer->setOutputCallback([this](const AudioBuffer& buffer) {
    //     router->routeAudio(buffer, mixer.get());
    // });
    
    // Connect mixer to audio engine
    // mixer->setOutputCallback([this](const AudioBuffer& buffer) {
    //     audioEngine->processBuffer(buffer);
    // });
    
    // Connect plugin host to mixer
    // pluginHost->setMixer(mixer.get());
    
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
    // if (sequencer) sequencer->shutdown();
    // if (router) router->shutdown();
    if (mixer) mixer->shutdown();
    // if (pluginHost) pluginHost->shutdown();
    // if (midiSequencer) midiSequencer->shutdown();
    // if (midiDevice) midiDevice->shutdown();
    if (audioEngine) audioEngine->shutdown();
    
    initialized = false;
    
    std::cout << "Omega DAW shutdown complete" << std::endl;
}

bool DAWApplication::run() {
    if (!initialized) {
        std::cerr << "Cannot run - application not initialized" << std::endl;
        return false;
    }
    
    while (running && uiWindow->isRunning()) {
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
    
    // Get current playback position in seconds
    double position = transport->getPositionSeconds();
    double bufferDuration = audioEngine->getBufferSize() / static_cast<double>(audioEngine->getSampleRate());
    double startTime = position;
    double endTime = position + bufferDuration;
    
    // Process MIDI sequencer
    if (midiSequencer && midiSynth) {
        MIDIBuffer midiBuffer;
        midiSequencer->process(startTime, endTime, midiBuffer);
        
        // Send MIDI events to synthesizer
        midiSynth->processMIDIBuffer(midiBuffer);
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
    // midiDevice->processEvents(); // Commented out until MIDIDevice is fully implemented
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
    std::string projectData;
    auto result = fileIO->loadProject(filepath, projectData);
    if (!result.success) {
        std::cerr << "Failed to load project: " << filepath << " - " << result.errorMessage << std::endl;
        return false;
    }
    
    // Parse basic project data from string
    // Format: "name:value\nname:value\n..."
    size_t pos = 0;
    while (pos < projectData.length()) {
        size_t endPos = projectData.find('\n', pos);
        if (endPos == std::string::npos) endPos = projectData.length();
        
        std::string line = projectData.substr(pos, endPos - pos);
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            if (key == "name") {
                project->setName(value);
            } else if (key == "tempo" && transport) {
                transport->setTempo(std::stod(value));
            }
        }
        
        pos = endPos + 1;
    }
    
    project->setFilePath(filepath);
    std::cout << "Project loaded: " << filepath << std::endl;
    return true;
}

bool DAWApplication::saveProject(const std::string& filepath) {
    // Gather current state into project
    project->setArrangementData(arrangement->serialize());
    project->setMixerData(mixer->serialize());
    
    // Serialize project to simple text format
    std::string projectData;
    projectData += "name:" + project->getName() + "\n";
    if (transport) {
        projectData += "tempo:" + std::to_string(transport->getTempo()) + "\n";
        projectData += "timesig:" + std::to_string(transport->getTimeSignatureNumerator()) + 
                      "/" + std::to_string(transport->getTimeSignatureDenominator()) + "\n";
    }
    projectData += "samplerate:" + std::to_string(project->getSampleRate()) + "\n";
    projectData += "buffersize:" + std::to_string(project->getBufferSize()) + "\n";
    
    auto result = fileIO->saveProject(filepath, projectData);
    if (!result.success) {
        std::cerr << "Failed to save project: " << filepath << " - " << result.errorMessage << std::endl;
        return false;
    }
    
    project->setFilePath(filepath);
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
