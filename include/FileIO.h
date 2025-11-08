#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace OmegaDAW {

enum class FileFormat {
    WAV,
    AIFF,
    FLAC,
    MP3,
    OGG,
    PROJECT,
    MIDI,
    UNKNOWN
};

enum class FileIOError {
    NONE,
    FILE_NOT_FOUND,
    INVALID_FORMAT,
    PERMISSION_DENIED,
    CORRUPT_DATA,
    UNSUPPORTED_FORMAT,
    DISK_FULL,
    UNKNOWN_ERROR
};

struct FileIOResult {
    bool success;
    FileIOError error;
    std::string errorMessage;
    
    FileIOResult() : success(true), error(FileIOError::NONE) {}
    FileIOResult(FileIOError err, const std::string& msg) 
        : success(false), error(err), errorMessage(msg) {}
};

class AudioFileReader {
public:
    AudioFileReader();
    ~AudioFileReader();
    
    FileIOResult open(const std::string& filepath);
    FileIOResult readSamples(float* buffer, size_t numSamples);
    FileIOResult readAllSamples(std::vector<std::vector<float>>& channels);
    
    int getSampleRate() const { return sampleRate; }
    int getNumChannels() const { return numChannels; }
    size_t getTotalSamples() const { return totalSamples; }
    FileFormat getFormat() const { return format; }
    
    void close();
    
private:
    FileFormat format;
    int sampleRate;
    int numChannels;
    size_t totalSamples;
    size_t currentPosition;
    void* fileHandle;
    
    FileFormat detectFormat(const std::string& filepath);
    FileIOResult readWAV(const std::string& filepath);
    FileIOResult readAIFF(const std::string& filepath);
    FileIOResult readFLAC(const std::string& filepath);
};

class AudioFileWriter {
public:
    AudioFileWriter();
    ~AudioFileWriter();
    
    FileIOResult open(const std::string& filepath, FileFormat format, 
                      int sampleRate, int numChannels, int bitDepth = 16);
    FileIOResult writeSamples(const float* buffer, size_t numSamples);
    FileIOResult writeAllSamples(const std::vector<std::vector<float>>& channels);
    
    void close();
    
private:
    FileFormat format;
    int sampleRate;
    int numChannels;
    int bitDepth;
    void* fileHandle;
    
    FileIOResult writeWAV(const std::string& filepath);
    FileIOResult writeAIFF(const std::string& filepath);
    FileIOResult writeFLAC(const std::string& filepath);
};

class ProjectFile {
public:
    ProjectFile();
    ~ProjectFile();
    
    FileIOResult save(const std::string& filepath);
    FileIOResult load(const std::string& filepath);
    FileIOResult autoSave();
    
    void setProjectData(const std::string& jsonData);
    std::string getProjectData() const;
    
    void enableAutoSave(bool enable, int intervalSeconds = 300);
    
private:
    std::string projectData;
    std::string lastSavedPath;
    bool autoSaveEnabled;
    int autoSaveInterval;
    
    FileIOResult saveJSON(const std::string& filepath, const std::string& data);
    FileIOResult loadJSON(const std::string& filepath, std::string& data);
};

class FileManager {
public:
    static FileManager& getInstance();
    
    FileIOResult importAudioFile(const std::string& filepath, std::vector<std::vector<float>>& audioData, int& sampleRate);
    FileIOResult exportAudioFile(const std::string& filepath, const std::vector<std::vector<float>>& audioData, 
                                 int sampleRate, FileFormat format = FileFormat::WAV);
    
    FileIOResult saveProject(const std::string& filepath, const std::string& projectData);
    FileIOResult loadProject(const std::string& filepath, std::string& projectData);
    
    FileIOResult exportMIDI(const std::string& filepath, const std::vector<uint8_t>& midiData);
    FileIOResult importMIDI(const std::string& filepath, std::vector<uint8_t>& midiData);
    
    std::vector<std::string> getRecentFiles() const;
    void addRecentFile(const std::string& filepath);
    void clearRecentFiles();
    
    std::string getFileExtension(const std::string& filepath);
    FileFormat getFileFormat(const std::string& filepath);
    bool fileExists(const std::string& filepath);
    
    void setProgressCallback(std::function<void(float)> callback);
    
private:
    FileManager();
    ~FileManager();
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;
    
    std::vector<std::string> recentFiles;
    std::function<void(float)> progressCallback;
    const int maxRecentFiles = 10;
    
    void notifyProgress(float progress);
};

} // namespace OmegaDAW
