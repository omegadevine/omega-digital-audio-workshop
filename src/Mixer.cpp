#include "Mixer.h"
#include <algorithm>
#include <cmath>

namespace OmegaDAW {

// MixerBus implementation

MixerBus::MixerBus(const std::string& name, ChannelType type)
    : name_(name)
    , type_(type)
    , id_(-1)
    , volume_(1.0f)
    , pan_(0.0f)
    , muted_(false)
    , soloed_(false) {
}

void MixerBus::process(AudioBuffer& buffer) {
    if (muted_) {
        buffer.clear();
        return;
    }

    for (auto& effect : effects_) {
        if (effect && effect->isEnabled()) {
            effect->process(buffer);
        }
    }

    buffer.applyGain(volume_);

    if (buffer.getNumChannels() >= 2 && std::abs(pan_) > 0.001f) {
        float leftGain = 1.0f;
        float rightGain = 1.0f;
        
        if (pan_ < 0.0f) {
            rightGain = 1.0f + pan_;
        } else {
            leftGain = 1.0f - pan_;
        }
        
        float* leftChannel = buffer.getWritePointer(0);
        float* rightChannel = buffer.getWritePointer(1);
        
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            leftChannel[i] *= leftGain;
            rightChannel[i] *= rightGain;
        }
    }
}

void MixerBus::reset() {
    for (auto& effect : effects_) {
        if (effect) {
            effect->reset();
        }
    }
}

void MixerBus::setVolume(float volume) {
    volume_ = std::max(0.0f, volume);
}

void MixerBus::setPan(float pan) {
    pan_ = std::max(-1.0f, std::min(1.0f, pan));
}

void MixerBus::setMute(bool mute) {
    muted_ = mute;
}

void MixerBus::setSolo(bool solo) {
    soloed_ = solo;
}

void MixerBus::addEffect(std::shared_ptr<Effect> effect) {
    if (effect) {
        effects_.push_back(effect);
    }
}

void MixerBus::removeEffect(int index) {
    if (index >= 0 && index < static_cast<int>(effects_.size())) {
        effects_.erase(effects_.begin() + index);
    }
}

std::shared_ptr<Effect> MixerBus::getEffect(int index) {
    if (index >= 0 && index < static_cast<int>(effects_.size())) {
        return effects_[index];
    }
    return nullptr;
}

void MixerBus::addSend(int targetBusId, float level) {
    sends_[targetBusId] = std::max(0.0f, level);
}

void MixerBus::removeSend(int targetBusId) {
    sends_.erase(targetBusId);
}

void MixerBus::setSendLevel(int targetBusId, float level) {
    if (sends_.find(targetBusId) != sends_.end()) {
        sends_[targetBusId] = std::max(0.0f, level);
    }
}

float MixerBus::getSendLevel(int targetBusId) const {
    auto it = sends_.find(targetBusId);
    if (it != sends_.end()) {
        return it->second;
    }
    return 0.0f;
}

// Mixer implementation

Mixer::Mixer()
    : nextBusId_(0)
    , masterBusId_(-1)
    , soloMode_(false)
    , sampleRate_(44100)
    , bufferSize_(512) {
    
    masterBusId_ = addBus("Master", ChannelType::Master);
}

void Mixer::initialize(int sampleRate, int bufferSize) {
    sampleRate_ = sampleRate;
    bufferSize_ = bufferSize;
    masterOutput_.setSize(2, bufferSize);
    
    for (auto& pair : busBuffers_) {
        pair.second.setSize(2, bufferSize);
    }
}

void Mixer::process() {
    masterOutput_.clear();
    
    for (auto& pair : busBuffers_) {
        pair.second.clear();
    }
    
    processRoutingGraph();
}

void Mixer::processRoutingGraph() {
    bool anySoloed = false;
    for (const auto& pair : buses_) {
        if (pair.second && pair.second->isSoloed()) {
            anySoloed = true;
            break;
        }
    }
    
    for (int busId : processingOrder_) {
        auto busIt = buses_.find(busId);
        if (busIt == buses_.end() || !busIt->second) {
            continue;
        }
        
        auto& bus = busIt->second;
        auto& buffer = busBuffers_[busId];
        
        if (anySoloed && !bus->isSoloed() && bus->getType() != ChannelType::Master) {
            buffer.clear();
            continue;
        }
        
        bus->process(buffer);
        
        const auto& sends = bus->getSends();
        for (const auto& send : sends) {
            int targetBusId = send.first;
            float sendLevel = send.second;
            
            auto targetIt = busBuffers_.find(targetBusId);
            if (targetIt != busBuffers_.end()) {
                targetIt->second.addFrom(buffer, sendLevel);
            }
        }
        
        if (busId != masterBusId_) {
            masterOutput_.addFrom(buffer, 1.0f);
        }
    }
    
    if (masterBusId_ >= 0) {
        auto masterBus = buses_[masterBusId_];
        if (masterBus) {
            masterBus->process(masterOutput_);
        }
    }
}

void Mixer::sortBusesTopologically() {
    processingOrder_.clear();
    
    for (const auto& pair : buses_) {
        if (pair.first != masterBusId_) {
            processingOrder_.push_back(pair.first);
        }
    }
    
    if (masterBusId_ >= 0) {
        processingOrder_.push_back(masterBusId_);
    }
}

void Mixer::reset() {
    for (auto& pair : buses_) {
        if (pair.second) {
            pair.second->reset();
        }
    }
    
    for (auto& pair : busBuffers_) {
        pair.second.clear();
    }
    
    masterOutput_.clear();
}

int Mixer::addBus(const std::string& name, ChannelType type) {
    int busId = nextBusId_++;
    auto bus = std::make_shared<MixerBus>(name, type);
    bus->setId(busId);
    
    buses_[busId] = bus;
    busBuffers_[busId] = AudioBuffer(2, bufferSize_);
    
    sortBusesTopologically();
    
    return busId;
}

void Mixer::removeBus(int busId) {
    if (busId == masterBusId_) {
        return;
    }
    
    buses_.erase(busId);
    busBuffers_.erase(busId);
    
    for (auto& pair : buses_) {
        if (pair.second) {
            pair.second->removeSend(busId);
        }
    }
    
    sortBusesTopologically();
}

std::shared_ptr<MixerBus> Mixer::getBus(int busId) {
    auto it = buses_.find(busId);
    if (it != buses_.end()) {
        return it->second;
    }
    return nullptr;
}

void Mixer::routeAudio(int sourceBusId, int targetBusId, float level) {
    auto sourceBus = getBus(sourceBusId);
    if (sourceBus) {
        sourceBus->addSend(targetBusId, level);
        sortBusesTopologically();
    }
}

void Mixer::removeRoute(int sourceBusId, int targetBusId) {
    auto sourceBus = getBus(sourceBusId);
    if (sourceBus) {
        sourceBus->removeSend(targetBusId);
        sortBusesTopologically();
    }
}

void Mixer::setBusInput(int busId, const AudioBuffer& buffer) {
    auto it = busBuffers_.find(busId);
    if (it != busBuffers_.end()) {
        it->second.copyFrom(buffer);
    }
}

AudioBuffer Mixer::getMasterOutput() {
    return masterOutput_;
}

void Mixer::setSoloMode(bool enabled) {
    soloMode_ = enabled;
}

std::shared_ptr<MixerBus> Mixer::getChannel(int index) {
    auto ids = getBusIds();
    if (index >= 0 && index < static_cast<int>(ids.size())) {
        return getBus(ids[index]);
    }
    return nullptr;
}

std::vector<int> Mixer::getBusIds() const {
    std::vector<int> ids;
    for (const auto& pair : buses_) {
        ids.push_back(pair.first);
    }
    return ids;
}

void Mixer::process(AudioBuffer& buffer) {
    // Process mixer with provided buffer
    processRoutingGraph();
}

void Mixer::shutdown() {
    // Shutdown mixer
    buses_.clear();
}

void Mixer::loadFromProject(Project* project) {
    // Load mixer from project
    if (project) {
        // Parse mixer data
    }
}

std::string Mixer::serialize() const {
    // Serialize mixer to string
    return "";
}

void Mixer::setOutputCallback(std::function<void(const AudioBuffer&)> callback) {
    outputCallback_ = callback;
}

} // namespace OmegaDAW
