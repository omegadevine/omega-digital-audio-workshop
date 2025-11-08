#include "FileIO.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cstdint>

namespace OmegaDAW {

// WAV file structures
struct WAVHeader {
    char riff[4];
    uint32_t fileSize;
    char wave[4];
    char fmt[4];
    uint32_t fmtSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4];
    uint32_t dataSize;
};

// AudioFileReader Implementation
AudioFileReader::AudioFileReader() 
    : format(FileFormat::UNKNOWN), sampleRate(0), numChannels(0), 
      totalSamples(0), currentPosition(0), fileHandle(nullptr) {}

AudioFileReader::~AudioFileReader() {
    close();
}

FileFormat AudioFileReader::detectFormat(const std::string& filepath) {
    size_t dotPos = filepath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return FileFormat::UNKNOWN;
    }
    
    std::string ext = filepath.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == "wav") return FileFormat::WAV;
    if (ext == "aiff" || ext == "aif") return FileFormat::AIFF;
    if (ext == "flac") return FileFormat::FLAC;
    if (ext == "mp3") return FileFormat::MP3;
    if (ext == "ogg") return FileFormat::OGG;
    if (ext == "mid" || ext == "midi") return FileFormat::MIDI;
    
    return FileFormat::UNKNOWN;
}

FileIOResult AudioFileReader::open(const std::string& filepath) {
    format = detectFormat(filepath);
    
    if (format == FileFormat::UNKNOWN) {
        return FileIOResult(FileIOError::UNSUPPORTED_FORMAT, "Unknown file format");
    }
    
    switch (format) {
        case FileFormat::WAV:
            return readWAV(filepath);
        case FileFormat::AIFF:
            return readAIFF(filepath);
        case FileFormat::FLAC:
            return readFLAC(filepath);
        default:
            return FileIOResult(FileIOError::UNSUPPORTED_FORMAT, "Format not yet implemented");
    }
}

FileIOResult AudioFileReader::readWAV(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return FileIOResult(FileIOError::FILE_NOT_FOUND, "Could not open file");
    }
    
    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));
    
    if (std::strncmp(header.riff, "RIFF", 4) != 0 || std::strncmp(header.wave, "WAVE", 4) != 0) {
        return FileIOResult(FileIOError::INVALID_FORMAT, "Invalid WAV file");
    }
    
    sampleRate = header.sampleRate;
    numChannels = header.numChannels;
    totalSamples = header.dataSize / (header.numChannels * (header.bitsPerSample / 8));
    currentPosition = 0;
    
    fileHandle = new std::ifstream(filepath, std::ios::binary);
    static_cast<std::ifstream*>(fileHandle)->seekg(sizeof(WAVHeader));
    
    return FileIOResult();
}

FileIOResult AudioFileReader::readAIFF(const std::string& filepath) {
    return FileIOResult(FileIOError::UNSUPPORTED_FORMAT, "AIFF format not yet implemented");
}

FileIOResult AudioFileReader::readFLAC(const std::string& filepath) {
    return FileIOResult(FileIOError::UNSUPPORTED_FORMAT, "FLAC format not yet implemented");
}

FileIOResult AudioFileReader::readSamples(float* buffer, size_t numSamples) {
    if (!fileHandle) {
        return FileIOResult(FileIOError::UNKNOWN_ERROR, "No file open");
    }
    
    std::ifstream* file = static_cast<std::ifstream*>(fileHandle);
    std::vector<int16_t> tempBuffer(numSamples);
    
    file->read(reinterpret_cast<char*>(tempBuffer.data()), numSamples * sizeof(int16_t));
    size_t samplesRead = file->gcount() / sizeof(int16_t);
    
    for (size_t i = 0; i < samplesRead; ++i) {
        buffer[i] = tempBuffer[i] / 32768.0f;
    }
    
    currentPosition += samplesRead;
    return FileIOResult();
}

FileIOResult AudioFileReader::readAllSamples(std::vector<std::vector<float>>& channels) {
    channels.resize(numChannels);
    for (auto& channel : channels) {
        channel.resize(totalSamples);
    }
    
    std::vector<float> interleavedBuffer(totalSamples * numChannels);
    FileIOResult result = readSamples(interleavedBuffer.data(), totalSamples * numChannels);
    
    if (!result.success) {
        return result;
    }
    
    for (size_t i = 0; i < totalSamples; ++i) {
        for (int ch = 0; ch < numChannels; ++ch) {
            channels[ch][i] = interleavedBuffer[i * numChannels + ch];
        }
    }
    
    return FileIOResult();
}

void AudioFileReader::close() {
    if (fileHandle) {
        delete static_cast<std::ifstream*>(fileHandle);
        fileHandle = nullptr;
    }
}

// AudioFileWriter Implementation
AudioFileWriter::AudioFileWriter() 
    : format(FileFormat::WAV), sampleRate(44100), numChannels(2), 
      bitDepth(16), fileHandle(nullptr) {}

AudioFileWriter::~AudioFileWriter() {
    close();
}

FileIOResult AudioFileWriter::open(const std::string& filepath, FileFormat fmt, 
                                   int sRate, int nChannels, int bDepth) {
    format = fmt;
    sampleRate = sRate;
    numChannels = nChannels;
    bitDepth = bDepth;
    
    switch (format) {
        case FileFormat::WAV:
            return writeWAV(filepath);
        case FileFormat::AIFF:
            return writeAIFF(filepath);
        case FileFormat::FLAC:
            return writeFLAC(filepath);
        default:
            return FileIOResult(FileIOError::UNSUPPORTED_FORMAT, "Format not supported for writing");
    }
}

FileIOResult AudioFileWriter::writeWAV(const std::string& filepath) {
    std::ofstream* file = new std::ofstream(filepath, std::ios::binary);
    if (!file->is_open()) {
        delete file;
        return FileIOResult(FileIOError::PERMISSION_DENIED, "Could not create file");
    }
    
    WAVHeader header;
    std::memcpy(header.riff, "RIFF", 4);
    header.fileSize = 0;
    std::memcpy(header.wave, "WAVE", 4);
    std::memcpy(header.fmt, "fmt ", 4);
    header.fmtSize = 16;
    header.audioFormat = 1;
    header.numChannels = numChannels;
    header.sampleRate = sampleRate;
    header.bitsPerSample = bitDepth;
    header.byteRate = sampleRate * numChannels * (bitDepth / 8);
    header.blockAlign = numChannels * (bitDepth / 8);
    std::memcpy(header.data, "data", 4);
    header.dataSize = 0;
    
    file->write(reinterpret_cast<char*>(&header), sizeof(WAVHeader));
    
    fileHandle = file;
    return FileIOResult();
}

FileIOResult AudioFileWriter::writeAIFF(const std::string& filepath) {
    return FileIOResult(FileIOError::UNSUPPORTED_FORMAT, "AIFF writing not yet implemented");
}

FileIOResult AudioFileWriter::writeFLAC(const std::string& filepath) {
    return FileIOResult(FileIOError::UNSUPPORTED_FORMAT, "FLAC writing not yet implemented");
}

FileIOResult AudioFileWriter::writeSamples(const float* buffer, size_t numSamples) {
    if (!fileHandle) {
        return FileIOResult(FileIOError::UNKNOWN_ERROR, "No file open");
    }
    
    std::ofstream* file = static_cast<std::ofstream*>(fileHandle);
    std::vector<int16_t> tempBuffer(numSamples);
    
    for (size_t i = 0; i < numSamples; ++i) {
        float sample = std::max(-1.0f, std::min(1.0f, buffer[i]));
        tempBuffer[i] = static_cast<int16_t>(sample * 32767.0f);
    }
    
    file->write(reinterpret_cast<char*>(tempBuffer.data()), numSamples * sizeof(int16_t));
    
    return FileIOResult();
}

FileIOResult AudioFileWriter::writeAllSamples(const std::vector<std::vector<float>>& channels) {
    if (channels.empty()) {
        return FileIOResult(FileIOError::INVALID_FORMAT, "No audio data provided");
    }
    
    size_t numSamples = channels[0].size();
    std::vector<float> interleavedBuffer(numSamples * channels.size());
    
    for (size_t i = 0; i < numSamples; ++i) {
        for (size_t ch = 0; ch < channels.size(); ++ch) {
            interleavedBuffer[i * channels.size() + ch] = channels[ch][i];
        }
    }
    
    return writeSamples(interleavedBuffer.data(), interleavedBuffer.size());
}

void AudioFileWriter::close() {
    if (fileHandle) {
        std::ofstream* file = static_cast<std::ofstream*>(fileHandle);
        
        size_t currentPos = file->tellp();
        uint32_t dataSize = currentPos - sizeof(WAVHeader);
        uint32_t fileSize = currentPos - 8;
        
        file->seekp(4);
        file->write(reinterpret_cast<char*>(&fileSize), sizeof(uint32_t));
        
        file->seekp(40);
        file->write(reinterpret_cast<char*>(&dataSize), sizeof(uint32_t));
        
        file->close();
        delete file;
        fileHandle = nullptr;
    }
}

// ProjectFile Implementation
ProjectFile::ProjectFile() 
    : autoSaveEnabled(false), autoSaveInterval(300) {}

ProjectFile::~ProjectFile() {}

FileIOResult ProjectFile::save(const std::string& filepath) {
    FileIOResult result = saveJSON(filepath, projectData);
    if (result.success) {
        lastSavedPath = filepath;
    }
    return result;
}

FileIOResult ProjectFile::load(const std::string& filepath) {
    FileIOResult result = loadJSON(filepath, projectData);
    if (result.success) {
        lastSavedPath = filepath;
    }
    return result;
}

FileIOResult ProjectFile::autoSave() {
    if (!autoSaveEnabled || lastSavedPath.empty()) {
        return FileIOResult(FileIOError::UNKNOWN_ERROR, "Auto-save not configured");
    }
    
    std::string autoSavePath = lastSavedPath + ".autosave";
    return saveJSON(autoSavePath, projectData);
}

void ProjectFile::setProjectData(const std::string& jsonData) {
    projectData = jsonData;
}

std::string ProjectFile::getProjectData() const {
    return projectData;
}

void ProjectFile::enableAutoSave(bool enable, int intervalSeconds) {
    autoSaveEnabled = enable;
    autoSaveInterval = intervalSeconds;
}

FileIOResult ProjectFile::saveJSON(const std::string& filepath, const std::string& data) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return FileIOResult(FileIOError::PERMISSION_DENIED, "Could not create project file");
    }
    
    file << data;
    file.close();
    
    return FileIOResult();
}

FileIOResult ProjectFile::loadJSON(const std::string& filepath, std::string& data) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return FileIOResult(FileIOError::FILE_NOT_FOUND, "Could not open project file");
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    data = buffer.str();
    file.close();
    
    return FileIOResult();
}

// FileManager Implementation
FileManager& FileManager::getInstance() {
    static FileManager instance;
    return instance;
}

FileManager::FileManager() {}

FileManager::~FileManager() {}

FileIOResult FileManager::importAudioFile(const std::string& filepath, 
                                         std::vector<std::vector<float>>& audioData, 
                                         int& sampleRate) {
    AudioFileReader reader;
    FileIOResult result = reader.open(filepath);
    
    if (!result.success) {
        return result;
    }
    
    sampleRate = reader.getSampleRate();
    result = reader.readAllSamples(audioData);
    
    reader.close();
    
    if (result.success) {
        addRecentFile(filepath);
    }
    
    return result;
}

FileIOResult FileManager::exportAudioFile(const std::string& filepath, 
                                         const std::vector<std::vector<float>>& audioData, 
                                         int sampleRate, FileFormat format) {
    AudioFileWriter writer;
    FileIOResult result = writer.open(filepath, format, sampleRate, audioData.size());
    
    if (!result.success) {
        return result;
    }
    
    result = writer.writeAllSamples(audioData);
    writer.close();
    
    if (result.success) {
        addRecentFile(filepath);
    }
    
    return result;
}

FileIOResult FileManager::saveProject(const std::string& filepath, const std::string& projectData) {
    ProjectFile project;
    project.setProjectData(projectData);
    FileIOResult result = project.save(filepath);
    
    if (result.success) {
        addRecentFile(filepath);
    }
    
    return result;
}

FileIOResult FileManager::loadProject(const std::string& filepath, std::string& projectData) {
    ProjectFile project;
    FileIOResult result = project.load(filepath);
    
    if (result.success) {
        projectData = project.getProjectData();
        addRecentFile(filepath);
    }
    
    return result;
}

FileIOResult FileManager::exportMIDI(const std::string& filepath, const std::vector<uint8_t>& midiData) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return FileIOResult(FileIOError::PERMISSION_DENIED, "Could not create MIDI file");
    }
    
    file.write(reinterpret_cast<const char*>(midiData.data()), midiData.size());
    file.close();
    
    addRecentFile(filepath);
    return FileIOResult();
}

FileIOResult FileManager::importMIDI(const std::string& filepath, std::vector<uint8_t>& midiData) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return FileIOResult(FileIOError::FILE_NOT_FOUND, "Could not open MIDI file");
    }
    
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    midiData.resize(fileSize);
    file.read(reinterpret_cast<char*>(midiData.data()), fileSize);
    file.close();
    
    addRecentFile(filepath);
    return FileIOResult();
}

std::vector<std::string> FileManager::getRecentFiles() const {
    return recentFiles;
}

void FileManager::addRecentFile(const std::string& filepath) {
    auto it = std::find(recentFiles.begin(), recentFiles.end(), filepath);
    if (it != recentFiles.end()) {
        recentFiles.erase(it);
    }
    
    recentFiles.insert(recentFiles.begin(), filepath);
    
    if (recentFiles.size() > maxRecentFiles) {
        recentFiles.resize(maxRecentFiles);
    }
}

void FileManager::clearRecentFiles() {
    recentFiles.clear();
}

std::string FileManager::getFileExtension(const std::string& filepath) {
    size_t dotPos = filepath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "";
    }
    return filepath.substr(dotPos + 1);
}

FileFormat FileManager::getFileFormat(const std::string& filepath) {
    AudioFileReader reader;
    return reader.detectFormat(filepath);
}

bool FileManager::fileExists(const std::string& filepath) {
    std::ifstream file(filepath);
    return file.good();
}

void FileManager::setProgressCallback(std::function<void(float)> callback) {
    progressCallback = callback;
}

void FileManager::notifyProgress(float progress) {
    if (progressCallback) {
        progressCallback(progress);
    }
}

} // namespace OmegaDAW
