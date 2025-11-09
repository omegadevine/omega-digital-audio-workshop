#ifndef OMEGA_DAW_ROUTER_H
#define OMEGA_DAW_ROUTER_H

#include "AudioBuffer.h"
#include <memory>
#include <vector>
#include <map>
#include <set>

namespace OmegaDAW {

struct Connection {
    int sourceId;
    int sourceChannel;
    int targetId;
    int targetChannel;
    float gain;
    bool enabled;

    Connection(int srcId, int srcCh, int tgtId, int tgtCh, float g = 1.0f)
        : sourceId(srcId), sourceChannel(srcCh)
        , targetId(tgtId), targetChannel(tgtCh)
        , gain(g), enabled(true) {}
};

class AudioNode {
public:
    AudioNode(int id, int numInputs, int numOutputs);
    virtual ~AudioNode() = default;

    virtual void process() = 0;
    virtual void reset() = 0;

    int getId() const { return id_; }
    int getNumInputs() const { return numInputs_; }
    int getNumOutputs() const { return numOutputs_; }

    AudioBuffer& getInputBuffer(int channel);
    AudioBuffer& getOutputBuffer(int channel);

    void clearInputs();
    void clearOutputs();

protected:
    int id_;
    int numInputs_;
    int numOutputs_;
    
    std::vector<AudioBuffer> inputBuffers_;
    std::vector<AudioBuffer> outputBuffers_;
};

class Router {
public:
    Router();
    ~Router() = default;

    void initialize(int sampleRate, int bufferSize);
    void process();
    void reset();

    void addNode(std::shared_ptr<AudioNode> node);
    void removeNode(int nodeId);
    std::shared_ptr<AudioNode> getNode(int nodeId);

    void connect(int sourceId, int sourceChannel, int targetId, int targetChannel, float gain = 1.0f);
    void disconnect(int sourceId, int sourceChannel, int targetId, int targetChannel);
    void disconnectAll(int nodeId);

    void setConnectionGain(int sourceId, int sourceChannel, int targetId, int targetChannel, float gain);
    void setConnectionEnabled(int sourceId, int sourceChannel, int targetId, int targetChannel, bool enabled);

    std::vector<Connection> getConnectionsFrom(int nodeId) const;
    std::vector<Connection> getConnectionsTo(int nodeId) const;
    std::vector<Connection> getAllConnections() const;

    bool hasConnection(int sourceId, int sourceChannel, int targetId, int targetChannel) const;
    bool detectCycle(int sourceId, int targetId) const;

private:
    void updateProcessingOrder();
    void topologicalSort();
    bool hasCycleUtil(int nodeId, std::set<int>& visited, std::set<int>& recStack) const;

    std::map<int, std::shared_ptr<AudioNode>> nodes_;
    std::vector<Connection> connections_;
    std::vector<int> processingOrder_;
    
    int sampleRate_;
    int bufferSize_;
};

class InputNode : public AudioNode {
public:
    InputNode(int id, int numChannels);
    
    void process() override;
    void reset() override;
    
    void setInputBuffer(const AudioBuffer& buffer);
};

class OutputNode : public AudioNode {
public:
    OutputNode(int id, int numChannels);
    
    void process() override;
    void reset() override;
    
    AudioBuffer getOutputBuffer();
};

class GainNode : public AudioNode {
public:
    GainNode(int id);
    
    void process() override;
    void reset() override;
    
    void setGain(float gain) { gain_ = gain; }
    float getGain() const { return gain_; }

private:
    float gain_;
};

class PanNode : public AudioNode {
public:
    PanNode(int id);
    
    void process() override;
    void reset() override;
    
    void setPan(float pan);
    float getPan() const { return pan_; }

private:
    float pan_;
};

class MixNode : public AudioNode {
public:
    MixNode(int id, int numInputs);
    
    void process() override;
    void reset() override;
};

class SplitNode : public AudioNode {
public:
    SplitNode(int id, int numOutputs);
    
    void process() override;
    void reset() override;
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_ROUTER_H
