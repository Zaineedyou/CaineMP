#include "audio-engine.h"
#include <android/log.h>
#include <cstring>
#include <fstream>
#include <cstdint>
#include <inttypes.h>

#define TAG "AudioEngine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

#pragma pack(push, 1)
struct WavHeader {
    char riffId[4];
    uint32_t riffSize;
    char waveId[4];
};

struct WavFmtChunk {
    char chunkId[4];
    uint32_t chunkSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

struct WavDataChunk {
    char chunkId[4];
    uint32_t chunkSize;
};
#pragma pack(pop)

AudioEngine::AudioEngine()
    : sessionId_(0), sampleRate_(48000), channelCount_(2), framesPerBurst_(0) {
    LOGI("AudioEngine constructor");
    audioBuffer_.resize(AUDIO_BUFFER_SIZE * channelCount_);
}

AudioEngine::~AudioEngine() {
    LOGI("AudioEngine destructor");
    closeAudioStream();
}

bool AudioEngine::initialize() {
    LOGI("Initializing audio engine");
    if (createAudioStream() != Result::OK) {
        LOGE("Failed to create audio stream");
        return false;
    }
    sessionId_ = audioStream_->getSessionId();
    LOGI("Audio engine initialized, sessionId=%d", sessionId_);
    return true;
}

bool AudioEngine::playFile(const std::string& filePath) {
    LOGI("Playing file: %s", filePath.c_str());
    
    if (!decodeAudioFile(filePath)) {
        LOGE("Failed to decode file");
        return false;
    }
    
    if (!audioStream_) {
        LOGE("Audio stream not initialized");
        return false;
    }
    
    if (audioStream_->getState() == StreamState::Uninitialized) {
        LOGE("Stream uninitialized");
        return false;
    }
    
    Result result = audioStream_->requestStart();
    if (result != Result::OK) {
        LOGE("Failed to start stream: %s", convertToText(result));
        return false;
    }
    
    isPlaying_.store(true);
    LOGI("Playback started");
    return true;
}

void AudioEngine::pausePlayback() {
    if (!audioStream_) return;
    audioStream_->requestPause();
    isPlaying_.store(false);
    LOGI("Playback paused");
}

void AudioEngine::stopPlayback() {
    if (!audioStream_) return;
    audioStream_->requestStop();
    isPlaying_.store(false);
    audioBufferPosition_.store(0);
    LOGI("Playback stopped");
}

void AudioEngine::seekTo(int64_t positionMs) {
    int64_t framesToSeek = (positionMs * sampleRate_) / 1000;
    audioBufferPosition_.store(framesToSeek);
    LOGI("Seek to %" PRId64 " ms", positionMs);
}

int64_t AudioEngine::getCurrentPosition() {
    int64_t pos = audioBufferPosition_.load();
    return (pos * 1000) / sampleRate_;
}

int64_t AudioEngine::getDuration() {
    return (audioBufferSize_ * 1000) / (sampleRate_ * channelCount_ * sizeof(float));
}

std::string AudioEngine::getStreamInfo() {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "SR:%dHz|Ch:%d|Playing:%s",
             sampleRate_, channelCount_, isPlaying_.load() ? "Yes" : "No");
    return std::string(buffer);
}

int32_t AudioEngine::getSessionId() {
    return sessionId_;
}

DataCallbackResult AudioEngine::onAudioReady(AudioStream* audioStream, void* audioData, int32_t numFrames) {
    if (!isPlaying_.load()) {
        memset(audioData, 0, numFrames * channelCount_ * sizeof(float));
        return DataCallbackResult::Continue;
    }
    
    float* outputBuffer = static_cast<float*>(audioData);
    int64_t pos = audioBufferPosition_.load();
    int64_t maxPos = audioBufferSize_ / (channelCount_ * sizeof(float));
    
    for (int32_t i = 0; i < numFrames; i++) {
        if (pos + i >= maxPos) {
            isPlaying_.store(false);
            memset(outputBuffer + i * channelCount_, 0, (numFrames - i) * channelCount_ * sizeof(float));
            return DataCallbackResult::Stop;
        }
        
        int64_t bufferIdx = (pos + i) * channelCount_;
        for (int ch = 0; ch < channelCount_; ch++) {
            outputBuffer[i * channelCount_ + ch] = audioBuffer_[bufferIdx + ch];
        }
    }
    
    audioBufferPosition_.store(pos + numFrames);
    return DataCallbackResult::Continue;
}

void AudioEngine::onErrorBeforeClose(AudioStream* audioStream, Result error) {
    LOGE("Audio stream error before close: %s", convertToText(error));
    isPlaying_.store(false);
}

void AudioEngine::onErrorAfterClose(AudioStream* audioStream, Result error) {
    LOGE("Audio stream error after close: %s", convertToText(error));
    isPlaying_.store(false);
}

Result AudioEngine::createAudioStream() {
    LOGI("Creating AAudio stream");
    
    AudioStreamBuilder builder;
    builder.setDirection(Direction::Output);
    builder.setPerformanceMode(PerformanceMode::LowLatency);
    builder.setFormat(AudioFormat::Float);
    builder.setChannelCount(channelCount_);
    builder.setSampleRate(sampleRate_);
    builder.setUsage(Usage::Media);
    builder.setContentType(ContentType::Music);
    builder.setCallback(this);
    
    builder.setSharingMode(SharingMode::Exclusive);
    Result result = builder.openStream(audioStream_);
    if (result == Result::OK) {
        LOGI("Exclusive mode OK");
    } else {
        LOGD("Exclusive failed, trying Shared: %s", convertToText(result));
        builder.setSharingMode(SharingMode::Shared);
        result = builder.openStream(audioStream_);
        if (result != Result::OK) {
            LOGE("Shared mode failed: %s", convertToText(result));
            return result;
        }
        LOGI("Shared mode OK");
    }
    
    sampleRate_ = audioStream_->getSampleRate();
    channelCount_ = audioStream_->getChannelCount();
    framesPerBurst_ = audioStream_->getFramesPerBurst();
    LOGI("Stream: SR=%d Hz, Ch=%d", sampleRate_, channelCount_);
    return Result::OK;
}

void AudioEngine::closeAudioStream() {
    LOGI("Closing audio stream");
    if (audioStream_) {
        audioStream_->close();
        audioStream_.reset();
    }
}

bool AudioEngine::decodeAudioFile(const std::string& filePath) {
    if (filePath.length() > 4) {
        std::string ext = filePath.substr(filePath.length() - 4);
        if (ext == ".mp3" || ext == ".MP3") {
            return decodeMp3File(filePath);
        }
    }
    return decodeWavFile(filePath);
}

bool AudioEngine::decodeMp3File(const std::string& filePath) {
    LOGI("Decoding MP3: %s", filePath.c_str());
    
    AMediaExtractor* extractor = AMediaExtractor_new();
    if (!extractor) {
        LOGE("Failed to create extractor");
        return false;
    }
    
    if (AMediaExtractor_setDataSource(extractor, filePath.c_str()) != AMEDIA_OK) {
        LOGE("Failed to set data source");
        AMediaExtractor_delete(extractor);
        return false;
    }
    
    size_t trackCount = AMediaExtractor_getTrackCount(extractor);
    int audioTrack = -1;
    
    for (size_t i = 0; i < trackCount; i++) {
        AMediaFormat* format = AMediaExtractor_getTrackFormat(extractor, i);
        const char* mime = nullptr;
        AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime);
        
        if (mime && strstr(mime, "audio")) {
            audioTrack = i;
            AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, &sampleRate_);
            AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &channelCount_);
            AMediaFormat_delete(format);
            break;
        }
        AMediaFormat_delete(format);
    }
    
    if (audioTrack < 0) {
        LOGE("No audio track found");
        AMediaExtractor_delete(extractor);
        return false;
    }
    
    AMediaExtractor_selectTrack(extractor, audioTrack);
    
    AMediaFormat* format = AMediaExtractor_getTrackFormat(extractor, audioTrack);
    AMediaCodec* codec = AMediaCodec_createDecoderByType("audio/mpeg");
    if (!codec) {
        LOGE("Failed to create decoder");
        AMediaFormat_delete(format);
        AMediaExtractor_delete(extractor);
        return false;
    }
    
    AMediaCodec_configure(codec, format, nullptr, nullptr, 0);
    AMediaCodec_start(codec);
    
    audioBuffer_.clear();
    audioBuffer_.resize(AUDIO_BUFFER_SIZE * channelCount_);
    audioBufferSize_ = 0;
    
    bool eos = false;
    while (!eos && audioBufferSize_ < AUDIO_BUFFER_SIZE * channelCount_) {
        ssize_t inputIdx = AMediaCodec_dequeueInputBuffer(codec, 2000);
        if (inputIdx >= 0) {
            size_t bufSize = 0;
            uint8_t* buf = AMediaCodec_getInputBuffer(codec, inputIdx, &bufSize);
            ssize_t sampleSize = AMediaExtractor_readSampleData(extractor, buf, bufSize);
            
            if (sampleSize <= 0) {
                AMediaCodec_queueInputBuffer(codec, inputIdx, 0, 0, 0, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
                eos = true;
            } else {
                AMediaCodec_queueInputBuffer(codec, inputIdx, 0, sampleSize, 
                                            AMediaExtractor_getSampleTime(extractor), 0);
                AMediaExtractor_advance(extractor);
            }
        }
        
        AMediaCodecBufferInfo info;
        ssize_t outputIdx = AMediaCodec_dequeueOutputBuffer(codec, &info, 2000);
        if (outputIdx >= 0) {
            size_t bufSize = 0;
            uint8_t* buf = AMediaCodec_getOutputBuffer(codec, outputIdx, &bufSize);
            
            size_t framesToCopy = info.size / (channelCount_ * sizeof(float));
            if (audioBufferSize_ + framesToCopy * channelCount_ <= AUDIO_BUFFER_SIZE * channelCount_) {
                memcpy(&audioBuffer_[audioBufferSize_], buf, info.size);
                audioBufferSize_ += framesToCopy * channelCount_;
            }
            
            AMediaCodec_releaseOutputBuffer(codec, outputIdx, false);
        }
    }
    
    AMediaCodec_stop(codec);
    AMediaCodec_delete(codec);
    AMediaFormat_delete(format);
    AMediaExtractor_delete(extractor);
    
    LOGI("MP3 decoded: %zu samples", audioBufferSize_ / channelCount_);
    return audioBufferSize_ > 0;
}

bool AudioEngine::decodeWavFile(const std::string& filePath) {
    LOGI("Decoding WAV: %s", filePath.c_str());
    
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOGE("Failed to open file");
        return false;
    }
    
    WavHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (strncmp(header.riffId, "RIFF", 4) != 0 || strncmp(header.waveId, "WAVE", 4) != 0) {
        LOGE("Invalid WAV file");
        return false;
    }
    
    WavFmtChunk fmt;
    file.read(reinterpret_cast<char*>(&fmt), sizeof(fmt));
    if (strncmp(fmt.chunkId, "fmt ", 4) != 0) {
        LOGE("Invalid fmt chunk");
        return false;
    }
    
    sampleRate_ = fmt.sampleRate;
    channelCount_ = fmt.numChannels;
    
    WavDataChunk data;
    file.read(reinterpret_cast<char*>(&data), sizeof(data));
    if (strncmp(data.chunkId, "data", 4) != 0) {
        LOGE("Invalid data chunk");
        return false;
    }
    
    audioBuffer_.resize(data.chunkSize / sizeof(float));
    file.read(reinterpret_cast<char*>(audioBuffer_.data()), data.chunkSize);
    audioBufferSize_ = audioBuffer_.size();
    audioBufferPosition_.store(0);
    
    LOGI("WAV decoded: %zu samples", audioBufferSize_ / channelCount_);
    return true;
}
