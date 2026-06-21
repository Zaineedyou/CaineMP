#include "audio-engine.h"
#include <android/log.h>
#include <cstring>
#include <fstream>
#include <cstdint>

#define TAG "AudioEngine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

// WAV file format structures
#pragma pack(push, 1)
struct WavHeader {
    char riffId[4];        // "RIFF"
    uint32_t riffSize;     // File size - 8
    char waveId[4];        // "WAVE"
};

struct WavFmtChunk {
    char chunkId[4];       // "fmt "
    uint32_t chunkSize;    // 16 for PCM
    uint16_t audioFormat;  // 1 for PCM
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

struct WavDataChunk {
    char chunkId[4];       // "data"
    uint32_t chunkSize;    // Data size in bytes
};
#pragma pack(pop)

// ==================== Constructor & Destructor ====================

AudioEngine::AudioEngine()
    : sessionId_(0),
      sampleRate_(48000),
      channelCount_(2),
      framesPerBurst_(0) {
    LOGI("AudioEngine constructor called");
    audioBuffer_.resize(AUDIO_BUFFER_SIZE * channelCount_);
}

AudioEngine::~AudioEngine() {
    LOGI("AudioEngine destructor called");
    stop();
    closeAudioStream();
}

// ==================== Public Methods ====================

int32_t AudioEngine::initialize() {
    LOGI("Initializing AudioEngine with AAudio Exclusive Mode");
    
    if (audioStream_ != nullptr) {
        LOGD("Audio stream already initialized");
        return sessionId_;
    }
    
    Result result = createAudioStream();
    if (result != Result::OK) {
        LOGE("Failed to create audio stream: %s", convertToText(result));
        return 0;
    }
    
    sessionId_ = audioStream_->getSessionId();
    LOGI("Audio engine initialized with session ID: %d", sessionId_);
    
    return sessionId_;
}

bool AudioEngine::playFile(const std::string& filePath) {
    LOGI("Playing file: %s", filePath.c_str());
    
    if (audioStream_ == nullptr) {
        LOGE("Audio stream not initialized");
        return false;
    }
    
    if (!decodeAudioFile(filePath)) {
        LOGE("Failed to decode audio file: %s", filePath.c_str());
        return false;
    }
    
    currentPositionMs_.store(0);
    bufferReadPos_.store(0);
    
    isPlaying_.store(true);
    isPaused_.store(false);
    
    Result result = audioStream_->requestStart();
    if (result != Result::OK) {
        LOGE("Failed to start audio stream: %s", convertToText(result));
        isPlaying_.store(false);
        return false;
    }
    
    LOGI("Playback started successfully");
    return true;
}

void AudioEngine::pause() {
    LOGI("Pausing playback");
    
    if (!isPlaying_.load()) {
        LOGD("Not currently playing");
        return;
    }
    
    isPaused_.store(true);
    
    if (audioStream_ != nullptr) {
        Result result = audioStream_->pause();
        if (result != Result::OK) {
            LOGE("Failed to pause audio stream: %s", convertToText(result));
        }
    }
}

void AudioEngine::resume() {
    LOGI("Resuming playback");
    
    if (!isPaused_.load()) {
        LOGD("Not paused");
        return;
    }
    
    isPaused_.store(false);
    
    if (audioStream_ != nullptr) {
        Result result = audioStream_->start();
        if (result != Result::OK) {
            LOGE("Failed to resume audio stream: %s", convertToText(result));
        }
    }
}

void AudioEngine::stop() {
    LOGI("Stopping playback");
    
    if (audioStream_ != nullptr) {
        audioStream_->requestStop();
    }
    
    isPlaying_.store(false);
    isPaused_.store(false);
    currentPositionMs_.store(0);
    bufferReadPos_.store(0);
    bufferWritePos_.store(0);
}

void AudioEngine::seekTo(int64_t positionMs) {
    LOGI("Seeking to position: %lld ms", positionMs);
    
    if (durationMs_.load() <= 0) {
        LOGD("No audio file loaded");
        return;
    }
    
    if (positionMs < 0) {
        positionMs = 0;
    } else if (positionMs > durationMs_.load()) {
        positionMs = durationMs_.load();
    }
    
    int64_t samplePosition = (positionMs * sampleRate_ * channelCount_) / 1000;
    
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        bufferReadPos_.store(samplePosition % audioBuffer_.size());
    }
    
    currentPositionMs_.store(positionMs);
    LOGI("Seek completed to %lld ms", positionMs);
}

int32_t AudioEngine::getSessionId() const {
    return sessionId_;
}

int64_t AudioEngine::getCurrentPosition() const {
    return currentPositionMs_.load();
}

int64_t AudioEngine::getDuration() const {
    return durationMs_.load();
}

bool AudioEngine::isPlaying() const {
    return isPlaying_.load() && !isPaused_.load();
}

std::string AudioEngine::getStreamInfo() const {
    if (audioStream_ == nullptr) {
        return "Stream not initialized";
    }
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "SR=%d Hz|Ch=%d|Fmt=%s|Mode=%s|Perf=%s|Burst=%d|SID=%d",
        audioStream_->getSampleRate(),
        audioStream_->getChannelCount(),
        convertToText(audioStream_->getFormat()),
        convertToText(audioStream_->getSharingMode()),
        convertToText(audioStream_->getPerformanceMode()),
        audioStream_->getFramesPerBurst(),
        sessionId_);
    
    return std::string(buffer);
}

// ==================== AudioStreamCallback Implementation ====================

DataCallbackResult AudioEngine::onAudioReady(
    AudioStream* audioStream,
    void* audioData,
    int32_t numFrames) {
    
    if (!isPlaying_.load() || isPaused_.load()) {
        std::memset(audioData, 0, numFrames * audioStream->getChannelCount() * sizeof(float));
        return DataCallbackResult::Continue;
    }
    
    int32_t framesWritten = fillAudioBuffer(
        static_cast<float*>(audioData),
        numFrames);
    
    int64_t positionMs = (bufferReadPos_.load() * 1000) / 
                         (sampleRate_ * channelCount_);
    currentPositionMs_.store(positionMs);
    
    if (framesWritten < numFrames) {
        float* buffer = static_cast<float*>(audioData);
        int32_t remainingFrames = numFrames - framesWritten;
        std::memset(
            buffer + (framesWritten * audioStream->getChannelCount()),
            0,
            remainingFrames * audioStream->getChannelCount() * sizeof(float));
        
        isPlaying_.store(false);
    }
    
    return DataCallbackResult::Continue;
}

void AudioEngine::onErrorAfterClose(AudioStream* audioStream, Result error) {
    LOGE("Audio stream error after close: %s", convertToText(error));
    isPlaying_.store(false);
}

// ==================== Private Helper Methods ====================

Result AudioEngine::createAudioStream() {
    LOGI("Creating AAudio stream with Exclusive Mode");
    
    AudioStreamBuilder builder;
    
    builder.setDirection(Direction::Output);
    builder.setSharingMode(SharingMode::Exclusive);
    builder.setPerformanceMode(PerformanceMode::LowLatency);
    builder.setFormat(AudioFormat::Float);
    builder.setChannelCount(channelCount_);
    builder.setSampleRate(sampleRate_);
    builder.setUsage(Usage::Media);
    builder.setContentType(ContentType::Music);
    builder.setCallback(this);
    
    Result result = builder.openStream(&audioStream_);
    if (result != Result::OK) {
        LOGE("Failed to open audio stream: %s", convertToText(result));
        return result;
    }
    
    sampleRate_ = audioStream_->getSampleRate();
    channelCount_ = audioStream_->getChannelCount();
    framesPerBurst_ = audioStream_->getFramesPerBurst();
    
    LOGI("Audio stream created: SR=%d Hz, Ch=%d", sampleRate_, channelCount_);
    
    return Result::OK;
}

void AudioEngine::closeAudioStream() {
    LOGI("Closing audio stream");
    
    if (audioStream_ != nullptr) {
        audioStream_->close();
        audioStream_.reset();
    }
    
    sessionId_ = 0;
}

bool AudioEngine::decodeAudioFile(const std::string& filePath) {
    LOGI("Decoding WAV file: %s", filePath.c_str());
    
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOGE("Failed to open file: %s", filePath.c_str());
        return false;
    }
    
    // Read RIFF header
    WavHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));
    
    if (!file || std::strncmp(header.riffId, "RIFF", 4) != 0 || 
        std::strncmp(header.waveId, "WAVE", 4) != 0) {
        LOGE("Invalid WAV file format");
        file.close();
        return false;
    }
    
    LOGI("Valid RIFF/WAVE header found");
    
    // Find fmt chunk
    WavFmtChunk fmtChunk;
    bool fmtFound = false;
    
    while (file.read(reinterpret_cast<char*>(&fmtChunk), 8)) {
        if (std::strncmp(fmtChunk.chunkId, "fmt ", 4) == 0) {
            fmtFound = true;
            uint32_t fmtSize = fmtChunk.chunkSize;
            file.read(reinterpret_cast<char*>(&fmtChunk) + 8, 
                     std::min(fmtSize, (uint32_t)sizeof(WavFmtChunk) - 8));
            break;
        } else {
            // Skip unknown chunk
            file.seekg(fmtChunk.chunkSize, std::ios::cur);
        }
    }
    
    if (!fmtFound) {
        LOGE("fmt chunk not found");
        file.close();
        return false;
    }
    
    if (fmtChunk.audioFormat != 1) {
        LOGE("Only PCM audio format supported (format=%d)", fmtChunk.audioFormat);
        file.close();
        return false;
    }
    
    sampleRate_ = fmtChunk.sampleRate;
    channelCount_ = fmtChunk.numChannels;
    uint16_t bitsPerSample = fmtChunk.bitsPerSample;
    
    LOGI("WAV Format: SR=%d Hz, Ch=%d, BPS=%d", sampleRate_, channelCount_, bitsPerSample);
    
    if (bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32) {
        LOGE("Unsupported bits per sample: %d", bitsPerSample);
        file.close();
        return false;
    }
    
    // Find data chunk
    WavDataChunk dataChunk;
    bool dataFound = false;
    
    while (file.read(reinterpret_cast<char*>(&dataChunk), 8)) {
        if (std::strncmp(dataChunk.chunkId, "data", 4) == 0) {
            dataFound = true;
            break;
        } else {
            // Skip unknown chunk
            file.seekg(dataChunk.chunkSize, std::ios::cur);
        }
    }
    
    if (!dataFound) {
        LOGE("data chunk not found");
        file.close();
        return false;
    }
    
    uint32_t dataSize = dataChunk.chunkSize;
    uint32_t numSamples = dataSize / (bitsPerSample / 8);
    uint32_t numFrames = numSamples / channelCount_;
    
    LOGI("WAV Data: %u bytes, %u samples, %u frames", dataSize, numSamples, numFrames);
    
    // Resize buffer to fit audio data
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        audioBuffer_.resize(numSamples);
        bufferWritePos_.store(0);
        bufferReadPos_.store(0);
    }
    
    // Read and convert PCM data to float
    std::vector<uint8_t> rawData(dataSize);
    file.read(reinterpret_cast<char*>(rawData.data()), dataSize);
    
    if (!file) {
        LOGE("Failed to read audio data");
        file.close();
        return false;
    }
    
    file.close();
    
    // Convert PCM to float
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        
        uint32_t sampleIndex = 0;
        uint32_t byteIndex = 0;
        uint32_t bytesPerSample = bitsPerSample / 8;
        
        while (byteIndex < dataSize && sampleIndex < audioBuffer_.size()) {
            float sample = 0.0f;
            
            if (bitsPerSample == 16) {
                int16_t pcmSample = *reinterpret_cast<int16_t*>(&rawData[byteIndex]);
                sample = pcmSample / 32768.0f;  // Convert to [-1.0, 1.0]
            } else if (bitsPerSample == 24) {
                int32_t pcmSample = 0;
                pcmSample |= rawData[byteIndex];
                pcmSample |= (rawData[byteIndex + 1] << 8);
                pcmSample |= (rawData[byteIndex + 2] << 16);
                if (pcmSample & 0x800000) pcmSample |= 0xFF000000;  // Sign extend
                sample = pcmSample / 8388608.0f;  // Convert to [-1.0, 1.0]
            } else if (bitsPerSample == 32) {
                int32_t pcmSample = *reinterpret_cast<int32_t*>(&rawData[byteIndex]);
                sample = pcmSample / 2147483648.0f;  // Convert to [-1.0, 1.0]
            }
            
            audioBuffer_[sampleIndex] = sample;
            sampleIndex++;
            byteIndex += bytesPerSample;
        }
        
        bufferWritePos_.store(sampleIndex);
    }
    
    // Calculate duration
    int64_t durationMs = (numFrames * 1000) / sampleRate_;
    durationMs_.store(durationMs);
    
    LOGI("WAV decoded successfully: %lld ms duration", durationMs);
    return true;
}

int32_t AudioEngine::fillAudioBuffer(float* buffer, int32_t numFrames) {
    if (buffer == nullptr || numFrames <= 0) {
        return 0;
    }
    
    int32_t framesRead = 0;
    int32_t channelCount = audioStream_->getChannelCount();
    
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        
        size_t readPos = bufferReadPos_.load();
        size_t writePos = bufferWritePos_.load();
        
        for (int32_t frame = 0; frame < numFrames; frame++) {
            if (readPos >= writePos) {
                break;
            }
            
            for (int32_t ch = 0; ch < channelCount; ch++) {
                buffer[frame * channelCount + ch] = audioBuffer_[readPos + ch];
            }
            
            readPos += channelCount;
            framesRead++;
        }
        
        bufferReadPos_.store(readPos);
    }
    
    return framesRead;
}

float AudioEngine::getNextSample() {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    
    size_t readPos = bufferReadPos_.load();
    size_t writePos = bufferWritePos_.load();
    
    if (readPos >= writePos) {
        return 0.0f;
    }
    
    float sample = audioBuffer_[readPos];
    bufferReadPos_.store(readPos + 1);
    
    return sample;
}
