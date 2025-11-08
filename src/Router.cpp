#include "Router.h"
#include <algorithm>
#include <cmath>
#include <queue>

namespace OmegaDAW {

// AudioNode implementation

AudioNode::AudioNode(int id, int numInputs, int numOutputs)
    : id_(id)
    , numInputs_(numInputs)
    , numOutputs_(numOutputs) {
    
    inputBuffers_.resize(numInputs);
    outputBuffers_.resize(numOutputs);
}

AudioBuffer& AudioNode::getInputBuffer(int channel) {
    if (channel >= 0 && channel < numInputs_) {
        return inputBuffers_[channel];
    }
    static AudioBuffer dummy;
    return dummy;
}

AudioBuffer& AudioNode::getOutputBuffer(int channel) {
    if (channel >= 0 && channel < numOutputs_) {
        return outputBuffers_[channel];
    }
    static AudioBuffer dummy;
    return dummy;
}

void AudioNode::clearInputs() {
    for (auto& buffer : inputBuffers_) {
        buffer.clear();
    }
}

void AudioNode::clearOutputs() {
    for (auto& buffer : outputBuffers_) {
        buffer.clear();
    }
}

// Router implementation

Router::Router()
    : sampleRate_(44100)
    , bufferSize_(512) {
}

void Router::initialize(int sampleRate, int bufferSize) {
    sampleRate_ = sampleRate;
    bufferSize_ = bufferSize;
    
    for (auto& pair : nodes_) {
        if (pair.second) {
            for (int i = 0; i < pair.second->getNumInputs(); ++i) {
                pair.second->getInputBuffer(i).setSize(2, bufferSize);
            }
            for (int i = 0; i < pair.second->getNumOutputs(); ++i) {
                pair.second->getOutputBuffer(i).setSize(2, bufferSize);
            }
        }
    }
}

void Router::process() {
    for (int nodeId : processingOrder_) {
        auto it = nodes_.find(nodeId);
        if (it != nodes_.end() && it->second) {
            it->second->clearInputs();
            it->second->clearOutputs();
        }
    }
    
    for (int nodeId : processingOrder_) {
        auto it = nodes_.find(nodeId);
        if (it == nodes_.end() || !it->second) {
            continue;
        }
        
        auto& node = it->second;
        node->process();
        
        for (const auto& conn : connections_) {
            if (conn.sourceId == nodeId && conn.enabled) {
                auto targetIt = nodes_.find(conn.targetId);
                if (targetIt != nodes_.end() && targetIt->second) {
                    auto& sourceBuffer = node->getOutputBuffer(conn.sourceChannel);
                    auto& targetBuffer = targetIt->second->getInputBuffer(conn.targetChannel);
                    targetBuffer.addFrom(sourceBuffer, conn.gain);
                }
            }
        }
    }
}

void Router::reset() {
    for (auto& pair : nodes_) {
        if (pair.second) {
            pair.second->reset();
        }
    }
}

void Router::addNode(std::shared_ptr<AudioNode> node) {
    if (node) {
        nodes_[node->getId()] = node;
        
        for (int i = 0; i < node->getNumInputs(); ++i) {
            node->getInputBuffer(i).setSize(2, bufferSize_);
        }
        for (int i = 0; i < node->getNumOutputs(); ++i) {
            node->getOutputBuffer(i).setSize(2, bufferSize_);
        }
        
        updateProcessingOrder();
    }
}

void Router::removeNode(int nodeId) {
    nodes_.erase(nodeId);
    disconnectAll(nodeId);
    updateProcessingOrder();
}

std::shared_ptr<AudioNode> Router::getNode(int nodeId) {
    auto it = nodes_.find(nodeId);
    if (it != nodes_.end()) {
        return it->second;
    }
    return nullptr;
}

void Router::connect(int sourceId, int sourceChannel, int targetId, int targetChannel, float gain) {
    if (hasConnection(sourceId, sourceChannel, targetId, targetChannel)) {
        return;
    }
    
    if (detectCycle(sourceId, targetId)) {
        return;
    }
    
    connections_.emplace_back(sourceId, sourceChannel, targetId, targetChannel, gain);
    updateProcessingOrder();
}

void Router::disconnect(int sourceId, int sourceChannel, int targetId, int targetChannel) {
    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
            [=](const Connection& conn) {
                return conn.sourceId == sourceId && conn.sourceChannel == sourceChannel &&
                       conn.targetId == targetId && conn.targetChannel == targetChannel;
            }),
        connections_.end());
    
    updateProcessingOrder();
}

void Router::disconnectAll(int nodeId) {
    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
            [=](const Connection& conn) {
                return conn.sourceId == nodeId || conn.targetId == nodeId;
            }),
        connections_.end());
    
    updateProcessingOrder();
}

void Router::setConnectionGain(int sourceId, int sourceChannel, int targetId, int targetChannel, float gain) {
    for (auto& conn : connections_) {
        if (conn.sourceId == sourceId && conn.sourceChannel == sourceChannel &&
            conn.targetId == targetId && conn.targetChannel == targetChannel) {
            conn.gain = gain;
            break;
        }
    }
}

void Router::setConnectionEnabled(int sourceId, int sourceChannel, int targetId, int targetChannel, bool enabled) {
    for (auto& conn : connections_) {
        if (conn.sourceId == sourceId && conn.sourceChannel == sourceChannel &&
            conn.targetId == targetId && conn.targetChannel == targetChannel) {
            conn.enabled = enabled;
            break;
        }
    }
}

std::vector<Connection> Router::getConnectionsFrom(int nodeId) const {
    std::vector<Connection> result;
    for (const auto& conn : connections_) {
        if (conn.sourceId == nodeId) {
            result.push_back(conn);
        }
    }
    return result;
}

std::vector<Connection> Router::getConnectionsTo(int nodeId) const {
    std::vector<Connection> result;
    for (const auto& conn : connections_) {
        if (conn.targetId == nodeId) {
            result.push_back(conn);
        }
    }
    return result;
}

std::vector<Connection> Router::getAllConnections() const {
    return connections_;
}

bool Router::hasConnection(int sourceId, int sourceChannel, int targetId, int targetChannel) const {
    for (const auto& conn : connections_) {
        if (conn.sourceId == sourceId && conn.sourceChannel == sourceChannel &&
            conn.targetId == targetId && conn.targetChannel == targetChannel) {
            return true;
        }
    }
    return false;
}

bool Router::detectCycle(int sourceId, int targetId) const {
    std::set<int> visited;
    std::set<int> recStack;
    
    std::queue<int> q;
    q.push(targetId);
    
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        
        if (current == sourceId) {
            return true;
        }
        
        if (visited.find(current) != visited.end()) {
            continue;
        }
        visited.insert(current);
        
        for (const auto& conn : connections_) {
            if (conn.sourceId == current) {
                q.push(conn.targetId);
            }
        }
    }
    
    return false;
}

void Router::updateProcessingOrder() {
    topologicalSort();
}

void Router::topologicalSort() {
    processingOrder_.clear();
    
    std::map<int, int> inDegree;
    for (const auto& pair : nodes_) {
        inDegree[pair.first] = 0;
    }
    
    for (const auto& conn : connections_) {
        inDegree[conn.targetId]++;
    }
    
    std::queue<int> q;
    for (const auto& pair : inDegree) {
        if (pair.second == 0) {
            q.push(pair.first);
        }
    }
    
    while (!q.empty()) {
        int nodeId = q.front();
        q.pop();
        processingOrder_.push_back(nodeId);
        
        for (const auto& conn : connections_) {
            if (conn.sourceId == nodeId) {
                inDegree[conn.targetId]--;
                if (inDegree[conn.targetId] == 0) {
                    q.push(conn.targetId);
                }
            }
        }
    }
}

bool Router::hasCycleUtil(int nodeId, std::set<int>& visited, std::set<int>& recStack) const {
    visited.insert(nodeId);
    recStack.insert(nodeId);
    
    for (const auto& conn : connections_) {
        if (conn.sourceId == nodeId) {
            int neighbor = conn.targetId;
            
            if (recStack.find(neighbor) != recStack.end()) {
                return true;
            }
            
            if (visited.find(neighbor) == visited.end()) {
                if (hasCycleUtil(neighbor, visited, recStack)) {
                    return true;
                }
            }
        }
    }
    
    recStack.erase(nodeId);
    return false;
}

// InputNode implementation

InputNode::InputNode(int id, int numChannels)
    : AudioNode(id, 0, numChannels) {
}

void InputNode::process() {
}

void InputNode::reset() {
    clearOutputs();
}

void InputNode::setInputBuffer(const AudioBuffer& buffer) {
    if (numOutputs_ > 0) {
        outputBuffers_[0].copyFrom(buffer);
    }
}

// OutputNode implementation

OutputNode::OutputNode(int id, int numChannels)
    : AudioNode(id, numChannels, 0) {
}

void OutputNode::process() {
}

void OutputNode::reset() {
    clearInputs();
}

AudioBuffer OutputNode::getOutputBuffer() {
    if (numInputs_ > 0) {
        return inputBuffers_[0];
    }
    return AudioBuffer();
}

// GainNode implementation

GainNode::GainNode(int id)
    : AudioNode(id, 1, 1)
    , gain_(1.0f) {
}

void GainNode::process() {
    outputBuffers_[0].copyFrom(inputBuffers_[0]);
    outputBuffers_[0].applyGain(gain_);
}

void GainNode::reset() {
    clearInputs();
    clearOutputs();
}

// PanNode implementation

PanNode::PanNode(int id)
    : AudioNode(id, 1, 1)
    , pan_(0.0f) {
}

void PanNode::process() {
    outputBuffers_[0].copyFrom(inputBuffers_[0]);
    
    if (outputBuffers_[0].getNumChannels() >= 2 && std::abs(pan_) > 0.001f) {
        float leftGain = pan_ < 0.0f ? 1.0f : 1.0f - pan_;
        float rightGain = pan_ > 0.0f ? 1.0f : 1.0f + pan_;
        
        float* left = outputBuffers_[0].getWritePointer(0);
        float* right = outputBuffers_[0].getWritePointer(1);
        
        for (int i = 0; i < outputBuffers_[0].getNumSamples(); ++i) {
            left[i] *= leftGain;
            right[i] *= rightGain;
        }
    }
}

void PanNode::reset() {
    clearInputs();
    clearOutputs();
}

void PanNode::setPan(float pan) {
    pan_ = std::max(-1.0f, std::min(1.0f, pan));
}

// MixNode implementation

MixNode::MixNode(int id, int numInputs)
    : AudioNode(id, numInputs, 1) {
}

void MixNode::process() {
    outputBuffers_[0].clear();
    
    for (int i = 0; i < numInputs_; ++i) {
        outputBuffers_[0].addFrom(inputBuffers_[i], 1.0f);
    }
}

void MixNode::reset() {
    clearInputs();
    clearOutputs();
}

// SplitNode implementation

SplitNode::SplitNode(int id, int numOutputs)
    : AudioNode(id, 1, numOutputs) {
}

void SplitNode::process() {
    for (int i = 0; i < numOutputs_; ++i) {
        outputBuffers_[i].copyFrom(inputBuffers_[0]);
    }
}

void SplitNode::reset() {
    clearInputs();
    clearOutputs();
}

} // namespace OmegaDAW
