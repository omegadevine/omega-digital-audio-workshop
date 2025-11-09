#include "Project.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <cmath>

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
            
            // Save clips
            json clipsArray = json::array();
            for (const auto& clip : track->getClips()) {
                json clipJson;
                clipJson["type"] = static_cast<int>(clip->getType());
                clipJson["startTime"] = clip->getStartTime();
                clipJson["duration"] = clip->getDuration();
                clipJson["offset"] = clip->getOffset();
                clipJson["loop"] = clip->isLooping();
                clipJson["gain"] = clip->getGain();
                clipJson["name"] = clip->getName();
                clipJson["color"] = clip->getColor();
                
                if (clip->getType() == ClipType::Audio) {
                    auto audioClip = std::static_pointer_cast<AudioClip>(clip);
                    clipJson["sourceFile"] = audioClip->getSourceFile();
                    clipJson["pitch"] = audioClip->getPitch();
                    clipJson["reverse"] = audioClip->isReversed();
                } else if (clip->getType() == ClipType::MIDI) {
                    auto midiClip = std::static_pointer_cast<MIDIClip>(clip);
                    json notesArray = json::array();
                    for (const auto& note : midiClip->getNotes()) {
                        json noteJson;
                        noteJson["status"] = note.getStatus();
                        noteJson["data1"] = note.getData1();
                        noteJson["data2"] = note.getData2();
                        noteJson["timestamp"] = note.getTimestamp();
                        notesArray.push_back(noteJson);
                    }
                    clipJson["notes"] = notesArray;
                }
                
                clipsArray.push_back(clipJson);
            }
            trackJson["clips"] = clipsArray;
            
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
                
                // Load clips
                if (trackJson.contains("clips")) {
                    for (const auto& clipJson : trackJson["clips"]) {
                        ClipType type = static_cast<ClipType>(clipJson.value("type", 0));
                        double startTime = clipJson.value("startTime", 0.0);
                        double duration = clipJson.value("duration", 1.0);
                        
                        std::shared_ptr<Clip> clip;
                        
                        if (type == ClipType::Audio) {
                            auto audioClip = std::make_shared<AudioClip>(startTime, duration);
                            audioClip->setSourceFile(clipJson.value("sourceFile", ""));
                            audioClip->setPitch(clipJson.value("pitch", 0.0f));
                            audioClip->setReverse(clipJson.value("reverse", false));
                            clip = audioClip;
                        } else if (type == ClipType::MIDI) {
                            auto midiClip = std::make_shared<MIDIClip>(startTime, duration);
                            
                            if (clipJson.contains("notes")) {
                                for (const auto& noteJson : clipJson["notes"]) {
                                    MIDIMessage note(
                                        noteJson.value("status", 0),
                                        noteJson.value("data1", 0),
                                        noteJson.value("data2", 0)
                                    );
                                    note.setTimestamp(noteJson.value("timestamp", 0.0));
                                    midiClip->addNote(note);
                                }
                            }
                            clip = midiClip;
                        } else {
                            clip = std::make_shared<Clip>(type, startTime, duration);
                        }
                        
                        if (clip) {
                            clip->setOffset(clipJson.value("offset", 0.0));
                            clip->setLoop(clipJson.value("loop", false));
                            clip->setGain(clipJson.value("gain", 1.0f));
                            clip->setName(clipJson.value("name", "Clip"));
                            clip->setColor(clipJson.value("color", 0xFFFFFFFF));
                            
                            track->addClip(clip);
                        }
                    }
                }
                
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

void Project::createDemoClips() {
    // Create demo clips for visualization
    for (auto& track : tracks_) {
        if (track->getType() == TrackType::Audio) {
            // Add demo audio clips with synthesized audio
            int sampleRate = 44100;
            int trackIndex = track->getTrackIndex();
            
            // Clip 1: Sine wave
            auto clip1 = std::make_shared<AudioClip>(0.0, 2.0);
            clip1->setName("Sine Wave");
            clip1->setColor(0xFF4444FF);
            
            int numSamples = static_cast<int>(2.0 * sampleRate);
            auto audioBuffer1 = std::make_shared<AudioBuffer>(2, numSamples);
            float frequency1 = 440.0f + (trackIndex * 110.0f); // A4, B4, C#5
            
            for (int s = 0; s < numSamples; ++s) {
                float t = s / static_cast<float>(sampleRate);
                float sample = 0.3f * std::sin(2.0f * 3.14159265f * frequency1 * t);
                audioBuffer1->setSample(0, s, sample);
                audioBuffer1->setSample(1, s, sample);
            }
            clip1->setAudioData(audioBuffer1);
            track->addClip(clip1);

            // Clip 2: Square wave
            auto clip2 = std::make_shared<AudioClip>(3.0, 1.5);
            clip2->setName("Square Wave");
            clip2->setColor(0xFF44FF44);
            
            numSamples = static_cast<int>(1.5 * sampleRate);
            auto audioBuffer2 = std::make_shared<AudioBuffer>(2, numSamples);
            float frequency2 = 330.0f + (trackIndex * 82.5f); // E4, F#4, G#4
            
            for (int s = 0; s < numSamples; ++s) {
                float t = s / static_cast<float>(sampleRate);
                float phase = std::fmod(frequency2 * t, 1.0f);
                float sample = 0.2f * (phase < 0.5f ? 1.0f : -1.0f);
                audioBuffer2->setSample(0, s, sample);
                audioBuffer2->setSample(1, s, sample);
            }
            clip2->setAudioData(audioBuffer2);
            track->addClip(clip2);

            // Clip 3: Sawtooth wave
            auto clip3 = std::make_shared<AudioClip>(5.5, 3.0);
            clip3->setName("Sawtooth");
            clip3->setColor(0xFFFF4444);
            
            numSamples = static_cast<int>(3.0 * sampleRate);
            auto audioBuffer3 = std::make_shared<AudioBuffer>(2, numSamples);
            float frequency3 = 220.0f + (trackIndex * 55.0f); // A3, B3, C#4
            
            for (int s = 0; s < numSamples; ++s) {
                float t = s / static_cast<float>(sampleRate);
                float phase = std::fmod(frequency3 * t, 1.0f);
                float sample = 0.2f * (2.0f * phase - 1.0f);
                audioBuffer3->setSample(0, s, sample);
                audioBuffer3->setSample(1, s, sample);
            }
            clip3->setAudioData(audioBuffer3);
            track->addClip(clip3);
            
        } else if (track->getType() == TrackType::MIDI) {
            // Add a demo MIDI clip
            auto midiClip = std::make_shared<MIDIClip>(0.0, 4.0);
            midiClip->setName("MIDI Pattern");
            midiClip->setColor(0xFFFFAA44);

            // Add some demo notes (C major scale)
            std::vector<int> notes = {60, 62, 64, 65, 67, 69, 71, 72}; // C4 to C5
            for (size_t i = 0; i < notes.size(); ++i) {
                double time = i * 0.5; // Quarter note spacing
                MIDIMessage noteOn(0x90, notes[i], 100); // Note on, velocity 100
                noteOn.setTimestamp(time);
                midiClip->addNote(noteOn);

                MIDIMessage noteOff(0x80, notes[i], 0); // Note off
                noteOff.setTimestamp(time + 0.4); // Slightly shorter than spacing
                midiClip->addNote(noteOff);
            }

            track->addClip(midiClip);
        }
    }

    std::cout << "Demo clips created on all tracks" << std::endl;
}

} // namespace OmegaDAW
