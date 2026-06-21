package com.example.bitperfect.audio

/**
 * AudioPlayer: Kotlin wrapper for native audio engine via JNI.
 * 
 * This class provides a Kotlin interface to the C++ audio engine,
 * handling bit-perfect playback with AAudio Exclusive Mode.
 */
class AudioPlayer {
    
    companion object {
        // Load native library
        init {
            System.loadLibrary("audio-engine")
        }
    }

    private var sessionId: Int = 0
    private var isInitialized: Boolean = false

    /**
     * Initialize the audio player.
     * Must be called before any playback operations.
     * 
     * @return Audio session ID for AudioEffect binding, or 0 if failed
     */
    fun initialize(): Int {
        if (!isInitialized) {
            sessionId = nativeInit()
            isInitialized = sessionId > 0
        }
        return sessionId
    }

    /**
     * Play an audio file.
     * 
     * @param filePath Path to the audio file
     * @return true if playback started successfully
     */
    fun playFile(filePath: String): Boolean {
        if (!isInitialized) {
            throw IllegalStateException("AudioPlayer not initialized. Call initialize() first.")
        }
        return nativePlayFile(filePath)
    }

    /**
     * Pause current playback.
     */
    fun pause() {
        if (isInitialized) {
            nativePause()
        }
    }

    /**
     * Resume playback.
     */
    fun resume() {
        if (isInitialized) {
            nativeResume()
        }
    }

    /**
     * Stop playback.
     */
    fun stop() {
        if (isInitialized) {
            nativeStop()
        }
    }

    /**
     * Seek to a specific position in the audio file.
     * 
     * @param positionMs Position in milliseconds
     */
    fun seekTo(positionMs: Long) {
        if (isInitialized) {
            nativeSeekTo(positionMs)
        }
    }

    /**
     * Get current playback position.
     * 
     * @return Position in milliseconds
     */
    fun getCurrentPosition(): Long {
        if (!isInitialized) return 0
        return nativeGetCurrentPosition()
    }

    /**
     * Get total duration of current audio file.
     * 
     * @return Duration in milliseconds
     */
    fun getDuration(): Long {
        if (!isInitialized) return 0
        return nativeGetDuration()
    }

    /**
     * Check if audio is currently playing.
     * 
     * @return true if playing, false otherwise
     */
    fun isPlaying(): Boolean {
        if (!isInitialized) return false
        return nativeIsPlaying()
    }

    /**
     * Get audio session ID for AudioEffect binding.
     * 
     * @return Session ID, or 0 if not initialized
     */
    fun getSessionId(): Int {
        return sessionId
    }

    /**
     * Get stream configuration info (for debugging).
     * 
     * @return Stream info string
     */
    fun getStreamInfo(): String {
        if (!isInitialized) return "Audio engine not initialized"
        return nativeGetStreamInfo()
    }

    /**
     * Release audio player resources.
     * Should be called when the player is no longer needed.
     */
    fun release() {
        if (isInitialized) {
            nativeRelease()
            isInitialized = false
            sessionId = 0
        }
    }

    // ==================== Native Methods ====================
    // These methods are implemented in jni-bridge.cpp

    private external fun nativeInit(): Int
    private external fun nativePlayFile(filePath: String): Boolean
    private external fun nativePause()
    private external fun nativeResume()
    private external fun nativeStop()
    private external fun nativeSeekTo(positionMs: Long)
    private external fun nativeGetCurrentPosition(): Long
    private external fun nativeGetDuration(): Long
    private external fun nativeIsPlaying(): Boolean
    private external fun nativeGetSessionId(): Int
    private external fun nativeGetStreamInfo(): String
    private external fun nativeRelease()
}
