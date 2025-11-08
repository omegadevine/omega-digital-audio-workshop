#include "Plugin.h"
#include <algorithm>
#include <stdexcept>

namespace OmegaDAW {

Plugin::Plugin(const std::string& name, PluginType type)
    : name(name)
    , version("1.0.0")
    , vendor("OmegaDAW")
    , type(type)
    , bypassed(false)
    , enabled_(true)
    , sampleRate(44100)
    , maxBufferSize(512)
{
}

void Plugin::setParameter(const std::string& id, float value) {
    auto it = parameters.find(id);
    if (it != parameters.end()) {
        float clampedValue = std::max(it->second.minValue, 
                                      std::min(it->second.maxValue, value));
        it->second.value = clampedValue;
    }
}

float Plugin::getParameter(const std::string& id) const {
    auto it = parameters.find(id);
    if (it != parameters.end()) {
        return it->second.value;
    }
    return 0.0f;
}

std::vector<PluginParameter> Plugin::getParameters() const {
    std::vector<PluginParameter> params;
    for (const auto& pair : parameters) {
        params.push_back(pair.second);
    }
    return params;
}

void Plugin::addParameter(const PluginParameter& param) {
    parameters[param.id] = param;
}

} // namespace OmegaDAW
