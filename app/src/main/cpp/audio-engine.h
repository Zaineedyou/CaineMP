#ifndef BITPERFECT_AUDIO_ENGINE_H
#define BITPERFECT_AUDIO_ENGINE_H

#include <oboe/Oboe.h>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>

using namespace oboe;

// Audio buffer size for PCM data
constexpr int32_t AUDIO_BUFFER_SIZE = 4096;

/**
 * AudioEngine: High-performance audio engine with AAudio Exclusive Mode.
 * 
 * Features:
 * - AAudio Exclusive Mode (bypass Android AudioFlinger mixer)
 * - Low-latency audio streaming (PerformanceMode::LowLatency)
 * - Bit-perfect playback (AudioFormat::Float, 32-bit)
 * - AudioEffect session binding for RootlessJamesDSP
 * 
 * Thread-safe operations with atomic flags and mutex protection.
 */
class AudioEngine : public AudioStreamCallback {
public:
    AudioEngine();
    ~AudioEngine();

    /**
     * Initialize audio engine and create exclusive mode stream.
     * 
     * Creates an AAudio stream with:
     * - SharingMode::Exclusive (bypass mixer)
     * - PerformanceMode::LowLatency
     * - AudioFormat::Float (32-bit)
     * 
     * @return Audio session ID for AudioEffect binding (>0 if successful)
     */
    int32_t initialize();

    /**
     * Load and play an audio file.
     * 
     * @param filePath Path to the audio file (WAV, FLAC, etc.)
     * @return true if successful, false otherwise
     */
    bool playFile(const std::string& filePath);

    /**
     * Pause current playback without closing stream.
     */
    void pause();

    /**
     * Resume playback from paused state.
     */
    void resume();

    /**
     * Stop playback and close audio stream.
     */
    void stop();

    /**
     * Seek to a specific position in the audio file.
     * 
     * @param positionMs Position in milliseconds
     */
    void seekTo(int64_t positionMs);

    /**
     * Get current audio session ID.
     * 
     * @return Session ID for AudioEffect binding (0 if not initialized)
     */
    int32_t getSessionId() const;

    /**
     * Get current playback position in milliseconds.
     * 
     * @return Position in milliseconds
     */
    int64_t getCurrentPosition() const;

    /**
     * Get total duration of current file in milliseconds.
     * 
     * @return Duration in milliseconds (0 if no file loaded)
     */
    int64_t getDuration() const;

    /**
     * Check if audio is currently playing.
     * 
     * @return true if playing, false otherwise
     */
    bool isPlaying() const;

    /**
     * Get stream configuration info (for debugging).
     * 
     * @return Stream configuration string
     */
    std::string getStreamInfo() const;

    // ==================== AudioStreamCallback Implementation ====================

    /**
     * Called by Oboe when audio data is needed.
     * This is where we feed PCM data to the audio output.
     */
    DataCallbackResult onAudioReady(
        AudioStream* audioStream,
        void* audioData,
        int32_t numFrames) override;

    /**
     * Called when audio stream encounters an error after closing.
     */
    void onErrorAfterClose(AudioStream* audioStream, Result error) override;

private:
    // ==================== Audio Stream Management ====================
    std::shared_ptr<AudioStream> audioStream_;
    int32_t sessionId_;
    
    // ==================== Playback State ====================
    std::atomic<bool> isPlaying_{false};
    std::atomic<bool> isPaused_{false};
    std::atomic<int64_t> currentPositionMs_{0};
    std::atomic<int64_t> durationMs_{0};
    
    // ==================== Audio Buffer Management ====================
    std::vector<float> audioBuffer_;
    std::atomic<size_t> bufferReadPos_{0};
    std::atomic<size_t> bufferWritePos_{0};
    std::mutex bufferMutex_;
    
    // ==================== Stream Configuration ====================
    int32_t sampleRate_;
    int32_t channelCount_;
    int32_t framesPerBurst_;
    
    // ==================== Private Helper Methods ====================
    
    /**
     * Create and open AAudio stream with Exclusive Mode.
     * 
     * @return Result::OK if successful
     */
    Result createAudioStream();
    
    /**
     * Close and release audio stream.
     */
    void closeAudioStream();
    
    /**
     * Decode audio file and fill buffer (stub for Milestone 2).
     * 
     * @param filePath Path to audio file
     * @return true if successful
     */
    bool decodeAudioFile(const std::string& filePath);
    
    /**
     * Fill audio buffer with PCM data from file.
     * 
     * @param buffer Output buffer
     * @param numFrames Number of frames to fill
     * @return Number of frames actually filled
     */
    int32_t fillAudioBuffer(float* buffer, int32_t numFrames);
    
    /**
     * Get next audio sample from buffer (thread-safe).
     * 
     * @return Audio sample (0.0f if buffer empty)
     */
    float getNextSample();
};

#endif // BITPERFECT_AUDIO_ENGINE_H
