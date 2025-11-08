#ifndef OMEGA_DAW_PROJECT_H
#define OMEGA_DAW_PROJECT_H

#include "Track.h"
#include "Transport.h"
#include <string>
#include <vector>
#include <memory>

namespace OmegaDAW {

class Project {
public:
    Project(const std::string& name = "Untitled");
    ~Project() = default;

    bool save(const std::string& filepath);
    bool load(const std::string& filepath);
    
    void addTrack(std::shared_ptr<Track> track);
    void removeTrack(int index);
    std::shared_ptr<Track> getTrack(int index);
    int getNumTracks() const { return static_cast<int>(tracks_.size()); }
    
    void setName(const std::string& name) { name_ = name; }
    const std::string& getName() const { return name_; }
    
    void setFilePath(const std::string& path) { filepath_ = path; }
    const std::string& getFilePath() const { return filepath_; }
    
    Transport& getTransport() { return transport_; }
    const Transport& getTransport() const { return transport_; }
    
    bool isModified() const { return modified_; }
    void setModified(bool modified) { modified_ = modified; }
    
    void setSampleRate(int sampleRate);
    int getSampleRate() const { return sampleRate_; }
    
    void setBufferSize(int bufferSize);
    int getBufferSize() const { return bufferSize_; }
    
    // Integration methods
    void clear();
    void setArrangementData(const std::string& data);
    void setMixerData(const std::string& data);
    const std::string& getArrangementData() const { return arrangementData_; }
    const std::string& getMixerData() const { return mixerData_; }

private:
    std::string arrangementData_;
    std::string mixerData_;
    std::string name_;
    std::string filepath_;
    bool modified_;
    
    std::vector<std::shared_ptr<Track>> tracks_;
    Transport transport_;
    
    int sampleRate_;
    int bufferSize_;
};

} // namespace OmegaDAW

#endif // OMEGA_DAW_PROJECT_H
