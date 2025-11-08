#ifndef OMEGA_DAW_MIDI_DEVICE_H
#define OMEGA_DAW_MIDI_DEVICE_H

#include "MIDIMessage.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace omega {

struct MIDIDeviceInfo {
    std::string name;
    int id;
    bool isInput;
    bool isOutput;
};

class MIDIInputDevice {
public:
    MIDIInputDevice(const std::string& name, int deviceId);
    virtual ~MIDIInputDevice();
    
    virtual bool open();
    virtual void close();
    virtual bool isOpen() const { return isOpen_; }
    
    void setMessageCallback(std::function<void(const MIDIMessage&)> callback);
    
    std::string getName() const { return name_; }
    int getDeviceId() const { return deviceId_; }
    
protected:
    void handleMessage(const MIDIMessage& message);
    
private:
    std::string name_;
    int deviceId_;
    bool isOpen_;
    std::function<void(const MIDIMessage&)> messageCallback_;
};

class MIDIOutputDevice {
public:
    MIDIOutputDevice(const std::string& name, int deviceId);
    virtual ~MIDIOutputDevice();
    
    virtual bool open();
    virtual void close();
    virtual bool isOpen() const { return isOpen_; }
    
    virtual void sendMessage(const MIDIMessage& message);
    
    std::string getName() const { return name_; }
    int getDeviceId() const { return deviceId_; }
    
private:
    std::string name_;
    int deviceId_;
    bool isOpen_;
};

class MIDIDeviceManager {
public:
    static MIDIDeviceManager& getInstance();
    
    void scanDevices();
    
    std::vector<MIDIDeviceInfo> getInputDevices() const;
    std::vector<MIDIDeviceInfo> getOutputDevices() const;
    
    std::shared_ptr<MIDIInputDevice> openInputDevice(int deviceId);
    std::shared_ptr<MIDIOutputDevice> openOutputDevice(int deviceId);
    
    void closeAllDevices();
    
private:
    MIDIDeviceManager();
    ~MIDIDeviceManager();
    
    std::vector<MIDIDeviceInfo> inputDevices_;
    std::vector<MIDIDeviceInfo> outputDevices_;
    std::vector<std::shared_ptr<MIDIInputDevice>> openInputs_;
    std::vector<std::shared_ptr<MIDIOutputDevice>> openOutputs_;
};

} // namespace omega

#endif // OMEGA_DAW_MIDI_DEVICE_H
