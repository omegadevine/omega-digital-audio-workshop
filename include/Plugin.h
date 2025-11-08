#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace OmegaDAW {

enum class PluginType {
    Effect,
    Instrument,
    Analyzer,
    Utility
};

struct PluginParameter {
    std::string id;
    std::string name;
    float value;
    float minValue;
    float maxValue;
    float defaultValue;
    std::string unit;
    bool isAutomatable;
};

class Plugin {
public:
    Plugin(const std::string& name, PluginType type);
    virtual ~Plugin() = default;

    virtual void initialize(int sampleRate, int maxBufferSize) = 0;
    virtual void process(float** inputs, float** outputs, int numChannels, int numSamples) = 0;
    virtual void reset() = 0;

    std::string getName() const { return name; }
    PluginType getType() const { return type; }
    std::string getVersion() const { return version; }
    std::string getVendor() const { return vendor; }
    
    void setParameter(const std::string& id, float value);
    float getParameter(const std::string& id) const;
    std::vector<PluginParameter> getParameters() const;
    
    void setBypass(bool bypass) { isBypassed = bypass; }
    bool isBypassed() const { return isBypassed; }
    
    void setEnabled(bool enabled) { isEnabled = enabled; }
    bool isEnabled() const { return isEnabled; }

protected:
    void addParameter(const PluginParameter& param);
    
    std::string name;
    std::string version;
    std::string vendor;
    PluginType type;
    bool isBypassed;
    bool isEnabled;
    int sampleRate;
    int maxBufferSize;
    
    std::map<std::string, PluginParameter> parameters;
};

} // namespace OmegaDAW
