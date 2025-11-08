#include "MIDIDevice.h"
#include <iostream>

namespace omega {

// MIDIInputDevice Implementation
MIDIInputDevice::MIDIInputDevice(const std::string& name, int deviceId)
    : name_(name)
    , deviceId_(deviceId)
    , isOpen_(false) {
}

MIDIInputDevice::~MIDIInputDevice() {
    if (isOpen_) {
        close();
    }
}

bool MIDIInputDevice::open() {
    if (isOpen_) {
        return true;
    }
    
    // Platform-specific implementation would go here
    // For now, this is a stub that simulates success
    std::cout << "Opening MIDI input device: " << name_ << std::endl;
    isOpen_ = true;
    return true;
}

void MIDIInputDevice::close() {
    if (!isOpen_) {
        return;
    }
    
    std::cout << "Closing MIDI input device: " << name_ << std::endl;
    isOpen_ = false;
}

void MIDIInputDevice::setMessageCallback(std::function<void(const MIDIMessage&)> callback) {
    messageCallback_ = callback;
}

void MIDIInputDevice::handleMessage(const MIDIMessage& message) {
    if (messageCallback_) {
        messageCallback_(message);
    }
}

// MIDIOutputDevice Implementation
MIDIOutputDevice::MIDIOutputDevice(const std::string& name, int deviceId)
    : name_(name)
    , deviceId_(deviceId)
    , isOpen_(false) {
}

MIDIOutputDevice::~MIDIOutputDevice() {
    if (isOpen_) {
        close();
    }
}

bool MIDIOutputDevice::open() {
    if (isOpen_) {
        return true;
    }
    
    std::cout << "Opening MIDI output device: " << name_ << std::endl;
    isOpen_ = true;
    return true;
}

void MIDIOutputDevice::close() {
    if (!isOpen_) {
        return;
    }
    
    std::cout << "Closing MIDI output device: " << name_ << std::endl;
    isOpen_ = false;
}

void MIDIOutputDevice::sendMessage(const MIDIMessage& message) {
    if (!isOpen_) {
        std::cerr << "Cannot send MIDI message: device not open" << std::endl;
        return;
    }
    
    // Platform-specific implementation would go here
    // This is a stub for demonstration
}

// MIDIDeviceManager Implementation
MIDIDeviceManager::MIDIDeviceManager() {
    scanDevices();
}

MIDIDeviceManager::~MIDIDeviceManager() {
    closeAllDevices();
}

MIDIDeviceManager& MIDIDeviceManager::getInstance() {
    static MIDIDeviceManager instance;
    return instance;
}

void MIDIDeviceManager::scanDevices() {
    inputDevices_.clear();
    outputDevices_.clear();
    
    // Platform-specific device enumeration would go here
    // For now, add some virtual devices for testing
    
    MIDIDeviceInfo virtualInput;
    virtualInput.name = "Virtual MIDI Input";
    virtualInput.id = 0;
    virtualInput.isInput = true;
    virtualInput.isOutput = false;
    inputDevices_.push_back(virtualInput);
    
    MIDIDeviceInfo virtualOutput;
    virtualOutput.name = "Virtual MIDI Output";
    virtualOutput.id = 0;
    virtualOutput.isInput = false;
    virtualOutput.isOutput = true;
    outputDevices_.push_back(virtualOutput);
    
    std::cout << "Scanned MIDI devices: " << inputDevices_.size() 
              << " inputs, " << outputDevices_.size() << " outputs" << std::endl;
}

std::vector<MIDIDeviceInfo> MIDIDeviceManager::getInputDevices() const {
    return inputDevices_;
}

std::vector<MIDIDeviceInfo> MIDIDeviceManager::getOutputDevices() const {
    return outputDevices_;
}

std::shared_ptr<MIDIInputDevice> MIDIDeviceManager::openInputDevice(int deviceId) {
    for (const auto& info : inputDevices_) {
        if (info.id == deviceId) {
            auto device = std::make_shared<MIDIInputDevice>(info.name, deviceId);
            if (device->open()) {
                openInputs_.push_back(device);
                return device;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<MIDIOutputDevice> MIDIDeviceManager::openOutputDevice(int deviceId) {
    for (const auto& info : outputDevices_) {
        if (info.id == deviceId) {
            auto device = std::make_shared<MIDIOutputDevice>(info.name, deviceId);
            if (device->open()) {
                openOutputs_.push_back(device);
                return device;
            }
        }
    }
    return nullptr;
}

void MIDIDeviceManager::closeAllDevices() {
    for (auto& device : openInputs_) {
        device->close();
    }
    openInputs_.clear();
    
    for (auto& device : openOutputs_) {
        device->close();
    }
    openOutputs_.clear();
}

} // namespace omega
