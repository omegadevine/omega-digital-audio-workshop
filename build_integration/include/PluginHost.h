#pragma once

#include "Plugin.h"
#include <vector>
#include <memory>
#include <functional>

namespace OmegaDAW {

class PluginHost {
public:
    PluginHost();
    ~PluginHost();

    void initialize(int sampleRate, int maxBufferSize);
    
    void addPlugin(std::shared_ptr<Plugin> plugin);
    void removePlugin(size_t index);
    void movePlugin(size_t fromIndex, size_t toIndex);
    
    std::shared_ptr<Plugin> getPlugin(size_t index);
    size_t getPluginCount() const { return pluginChain.size(); }
    
    void processPluginChain(float** inputs, float** outputs, int numChannels, int numSamples);
    
    void clearPlugins();
    void resetAllPlugins();
    
    void setPluginBypass(size_t index, bool bypass);
    void setPluginEnabled(size_t index, bool enabled);

private:
    std::vector<std::shared_ptr<Plugin>> pluginChain;
    int sampleRate;
    int maxBufferSize;
    
    std::vector<std::vector<float>> intermediateBuffers;
    void allocateIntermediateBuffers(int numChannels, int numSamples);
};

} // namespace OmegaDAW
