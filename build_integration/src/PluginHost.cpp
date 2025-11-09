#include "PluginHost.h"
#include <algorithm>
#include <cstring>

namespace OmegaDAW {

PluginHost::PluginHost()
    : sampleRate(44100)
    , maxBufferSize(512)
{
}

PluginHost::~PluginHost() {
}

void PluginHost::initialize(int sampleRate, int maxBufferSize) {
    this->sampleRate = sampleRate;
    this->maxBufferSize = maxBufferSize;
}

void PluginHost::addPlugin(std::shared_ptr<Plugin> plugin) {
    if (plugin) {
        plugin->initialize(sampleRate, maxBufferSize);
        pluginChain.push_back(plugin);
    }
}

void PluginHost::removePlugin(size_t index) {
    if (index < pluginChain.size()) {
        pluginChain.erase(pluginChain.begin() + index);
    }
}

void PluginHost::movePlugin(size_t fromIndex, size_t toIndex) {
    if (fromIndex < pluginChain.size() && toIndex < pluginChain.size()) {
        auto plugin = pluginChain[fromIndex];
        pluginChain.erase(pluginChain.begin() + fromIndex);
        pluginChain.insert(pluginChain.begin() + toIndex, plugin);
    }
}

std::shared_ptr<Plugin> PluginHost::getPlugin(size_t index) {
    if (index < pluginChain.size()) {
        return pluginChain[index];
    }
    return nullptr;
}

void PluginHost::allocateIntermediateBuffers(int numChannels, int numSamples) {
    if (intermediateBuffers.size() != static_cast<size_t>(numChannels) ||
        intermediateBuffers[0].size() != static_cast<size_t>(numSamples)) {
        intermediateBuffers.clear();
        for (int i = 0; i < numChannels; ++i) {
            intermediateBuffers.push_back(std::vector<float>(numSamples, 0.0f));
        }
    }
}

void PluginHost::processPluginChain(float** inputs, float** outputs, int numChannels, int numSamples) {
    if (pluginChain.empty()) {
        for (int ch = 0; ch < numChannels; ++ch) {
            std::memcpy(outputs[ch], inputs[ch], numSamples * sizeof(float));
        }
        return;
    }
    
    allocateIntermediateBuffers(numChannels, numSamples);
    
    float** currentInput = inputs;
    float** currentOutput = new float*[numChannels];
    for (int ch = 0; ch < numChannels; ++ch) {
        currentOutput[ch] = intermediateBuffers[ch].data();
    }
    
    for (size_t i = 0; i < pluginChain.size(); ++i) {
        auto& plugin = pluginChain[i];
        
        if (!plugin->isEnabled() || plugin->isBypassed()) {
            for (int ch = 0; ch < numChannels; ++ch) {
                std::memcpy(currentOutput[ch], currentInput[ch], numSamples * sizeof(float));
            }
        } else {
            plugin->process(currentInput, currentOutput, numChannels, numSamples);
        }
        
        currentInput = currentOutput;
        
        if (i == pluginChain.size() - 1) {
            for (int ch = 0; ch < numChannels; ++ch) {
                std::memcpy(outputs[ch], currentOutput[ch], numSamples * sizeof(float));
            }
        }
    }
    
    delete[] currentOutput;
}

void PluginHost::clearPlugins() {
    pluginChain.clear();
}

void PluginHost::resetAllPlugins() {
    for (auto& plugin : pluginChain) {
        plugin->reset();
    }
}

void PluginHost::setPluginBypass(size_t index, bool bypass) {
    if (index < pluginChain.size()) {
        pluginChain[index]->setBypass(bypass);
    }
}

void PluginHost::setPluginEnabled(size_t index, bool enabled) {
    if (index < pluginChain.size()) {
        pluginChain[index]->setEnabled(enabled);
    }
}

} // namespace OmegaDAW
