#include "Project.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace OmegaDAW {

Project::Project(const std::string& name)
    : name_(name)
    , filepath_("")
    , modified_(false)
    , sampleRate_(44100)
    , bufferSize_(512) {
}

bool Project::save(const std::string& filepath) {
    try {
        json projectJson;
        
        // Save project metadata
        projectJson["name"] = name_;
        projectJson["sampleRate"] = sampleRate_;
        projectJson["bufferSize"] = bufferSize_;
        projectJson["version"] = "1.0";
        
        // Save tracks
        json tracksArray = json::array();
        for (const auto& track : tracks_) {
            json trackJson;
            trackJson["index"] = track->getTrackIndex();
            trackJson["name"] = track->getName();
            trackJson["muted"] = track->isMuted();
            trackJson["solo"] = track->isSoloed();
            trackJson["volume"] = track->getVolume();
            trackJson["pan"] = track->getPan();
            tracksArray.push_back(trackJson);
        }
        projectJson["tracks"] = tracksArray;
        
        // Save arrangement and mixer data
        projectJson["arrangementData"] = arrangementData_;
        projectJson["mixerData"] = mixerData_;
        
        // Save transport state
        json transportJson;
        transportJson["playing"] = transport_.isPlaying();
        transportJson["recording"] = transport_.isRecording();
        transportJson["looping"] = transport_.isLooping();
        transportJson["tempo"] = transport_.getTempo();
        projectJson["transport"] = transportJson;
        
        // Write to file
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to save project to: " << filepath << std::endl;
            return false;
        }
        
        file << projectJson.dump(4); // Pretty print with 4 spaces
        file.close();
        
        filepath_ = filepath;
        modified_ = false;
        
        std::cout << "Project saved: " << filepath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving project: " << e.what() << std::endl;
        return false;
    }
}

bool Project::load(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to load project from: " << filepath << std::endl;
            return false;
        }
        
        json projectJson;
        file >> projectJson;
        file.close();
        
        // Load project metadata
        name_ = projectJson.value("name", "Untitled");
        sampleRate_ = projectJson.value("sampleRate", 44100);
        bufferSize_ = projectJson.value("bufferSize", 512);
        
        // Load tracks
        tracks_.clear();
        if (projectJson.contains("tracks")) {
            for (const auto& trackJson : projectJson["tracks"]) {
                auto track = std::make_shared<Track>(
                    trackJson.value("name", "Track"),
                    TrackType::Audio
                );
                track->setTrackIndex(trackJson.value("index", 0));
                track->setMute(trackJson.value("muted", false));
                track->setSolo(trackJson.value("solo", false));
                track->setVolume(trackJson.value("volume", 1.0f));
                track->setPan(trackJson.value("pan", 0.0f));
                tracks_.push_back(track);
            }
        }
        
        // Load arrangement and mixer data
        arrangementData_ = projectJson.value("arrangementData", "");
        mixerData_ = projectJson.value("mixerData", "");
        
        // Load transport state
        if (projectJson.contains("transport")) {
            const auto& transportJson = projectJson["transport"];
            transport_.setTempo(transportJson.value("tempo", 120.0));
            transport_.setLooping(transportJson.value("looping", false));
        }
        
        filepath_ = filepath;
        modified_ = false;
        
        std::cout << "Project loaded: " << filepath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading project: " << e.what() << std::endl;
        return false;
    }
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

void Project::clear() {
    tracks_.clear();
    name_ = "Untitled";
    filepath_ = "";
    modified_ = false;
    arrangementData_ = "";
    mixerData_ = "";
}

void Project::setArrangementData(const std::string& data) {
    arrangementData_ = data;
    modified_ = true;
}

void Project::setMixerData(const std::string& data) {
    mixerData_ = data;
    modified_ = true;
}

} // namespace OmegaDAW
