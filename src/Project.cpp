#include "Project.h"
#include <fstream>
#include <iostream>

namespace omega {

Project::Project(const std::string& name)
    : name_(name)
    , filepath_("")
    , modified_(false)
    , sampleRate_(44100)
    , bufferSize_(512) {
}

bool Project::save(const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to save project to: " << filepath << std::endl;
        return false;
    }
    
    filepath_ = filepath;
    modified_ = false;
    
    std::cout << "Project saved: " << filepath << std::endl;
    return true;
}

bool Project::load(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to load project from: " << filepath << std::endl;
        return false;
    }
    
    filepath_ = filepath;
    modified_ = false;
    
    std::cout << "Project loaded: " << filepath << std::endl;
    return true;
}

void Project::addTrack(std::shared_ptr<Track> track) {
    if (track) {
        track->setTrackIndex(static_cast<int>(tracks_.size()));
        tracks_.push_back(track);
        modified_ = true;
    }
}

void Project::removeTrack(int index) {
    if (index >= 0 && index < static_cast<int>(tracks_.size())) {
        tracks_.erase(tracks_.begin() + index);
        
        for (int i = index; i < static_cast<int>(tracks_.size()); ++i) {
            tracks_[i]->setTrackIndex(i);
        }
        
        modified_ = true;
    }
}

std::shared_ptr<Track> Project::getTrack(int index) {
    if (index >= 0 && index < static_cast<int>(tracks_.size())) {
        return tracks_[index];
    }
    return nullptr;
}

void Project::setSampleRate(int sampleRate) {
    sampleRate_ = sampleRate;
    transport_.setSampleRate(sampleRate);
    modified_ = true;
}

void Project::setBufferSize(int bufferSize) {
    bufferSize_ = bufferSize;
    modified_ = true;
}

} // namespace omega
