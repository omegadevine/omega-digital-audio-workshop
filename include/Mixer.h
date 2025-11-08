#ifndef OMEGA_DAW_MIXER_H
#define OMEGA_DAW_MIXER_H

#include "AudioBuffer.h"
#include "MixerChannel.h"
#include <memory>
#include <vector>
#include <map>
#include <string>

namespace omega {

enum class ChannelType {
    Audio,
    Group,
    Aux,
    Master
};

class MixerBus {
public:
    MixerBus(const std::string& name, ChannelType type);
    ~MixerBus() = default;

    void process(AudioBuffer& buffer);
    void reset();

    void setVolume(float volume);
    float getVolume() const { return volume_; }

    void setPan(float pan);
    float getPan() const { return pan_; }

    void setMute(bool mute);
    bool isMuted() const { return muted_; }

    void setSolo(bool solo);
    bool isSoloed() const { return soloed_; }

    void addEffect(std::shared_ptr<Effect> effect);
    void removeEffect(int index);
    std::shared_ptr<Effect> getEffect(int index);
    int getNumEffects() const { return static_cast<int>(effects_.size()); }

    void addSend(int targetBusId, float level);
    void removeSend(int targetBusId);
    void setSendLevel(int targetBusId, float level);
    float getSendLevel(int targetBusId) const;

    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    ChannelType getType() const { return type_; }

    int getId() const { return id_; }
    void setId(int id) { id_ = id; }

    const std::map<int, float>& getSends() const { return sends_; }

private:
    std::string name_;
    ChannelType type_;
    int id_;
    
    float volume_;
    float pan_;
    bool muted_;
    bool soloed_;
    
    std::vector<std::shared_ptr<Effect>> effects_;
    std::map<int, float> sends_;
};

class Mixer {
public:
    Mixer();
    ~Mixer() = default;

    void initialize(int sampleRate, int bufferSize);
    void process();
    void reset();

    int addBus(const std::string& name, ChannelType type);
    void removeBus(int busId);
    std::shared_ptr<MixerBus> getBus(int busId);
    
    void routeAudio(int sourceBusId, int targetBusId, float level = 1.0f);
    void removeRoute(int sourceBusId, int targetBusId);

    void setBusInput(int busId, const AudioBuffer& buffer);
    AudioBuffer getMasterOutput();

    int getMasterBusId() const { return masterBusId_; }

    void setSoloMode(bool enabled);
    bool isSoloMode() const { return soloMode_; }

    std::vector<int> getBusIds() const;

private:
    void processRoutingGraph();
    void sortBusesTopologically();
    
    std::map<int, std::shared_ptr<MixerBus>> buses_;
    std::vector<int> processingOrder_;
    std::map<int, AudioBuffer> busBuffers_;
    
    int nextBusId_;
    int masterBusId_;
    bool soloMode_;
    
    int sampleRate_;
    int bufferSize_;
    AudioBuffer masterOutput_;
};

} // namespace omega

#endif // OMEGA_DAW_MIXER_H
