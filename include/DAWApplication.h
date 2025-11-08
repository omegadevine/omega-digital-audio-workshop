#ifndef DAW_APPLICATION_H
#define DAW_APPLICATION_H

#include "AudioEngine.h"
#include "MIDISequencer.h"
#include "PluginHost.h"
#include "Mixer.h"
#include "Router.h"
#include "Sequencer.h"
#include "Arrangement.h"
#include "Transport.h"
#include "Project.h"
#include "FileIO.h"
#include "UIWindow.h"
#include <memory>
#include <string>

namespace OmegaDAW {

class DAWApplication {
public:
    DAWApplication();
    ~DAWApplication();
    
    bool initialize();
    bool initialize(int sampleRate, int bufferSize);
    void shutdown();
    
    bool run();
    void processAudio();
    
    // Component Access
    AudioEngine* getAudioEngine() { return audioEngine.get(); }
    MIDISequencer* getMIDISequencer() { return midiSequencer.get(); }
    PluginHost* getPluginHost() { return pluginHost.get(); }
    Mixer* getMixer() { return mixer.get(); }
    Router* getRouter() { return router.get(); }
    Sequencer* getSequencer() { return sequencer.get(); }
    Arrangement* getArrangement() { return arrangement.get(); }
    Transport* getTransport() { return transport.get(); }
    Project* getProject() { return project.get(); }
    FileManager* getFileIO() { return fileIO; }
    UIWindow* getUIWindow() { return uiWindow.get(); }
    
    // Project Operations
    bool newProject(const std::string& projectName);
    bool loadProject(const std::string& filepath);
    bool saveProject(const std::string& filepath);
    
    // Playback Control
    void play();
    void stop();
    void pause();
    void record();
    
    // Status
    bool isRunning() const { return running; }
    bool isPlaying() const;
    bool isRecording() const;
    
private:
    void connectComponents();
    void updateUI();
    void processEvents();
    
    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<MIDISequencer> midiSequencer;
    std::unique_ptr<PluginHost> pluginHost;
    std::unique_ptr<Mixer> mixer;
    std::unique_ptr<Router> router;
    std::unique_ptr<Sequencer> sequencer;
    std::unique_ptr<Arrangement> arrangement;
    std::unique_ptr<Transport> transport;
    std::unique_ptr<Project> project;
    FileManager* fileIO;
    std::unique_ptr<UIWindow> uiWindow;
    
    bool running;
    bool initialized;
};

} // namespace OmegaDAW

#endif // DAW_APPLICATION_H
