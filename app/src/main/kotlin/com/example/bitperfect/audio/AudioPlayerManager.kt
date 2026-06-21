package com.example.bitperfect.audio

import android.content.Context
import android.util.Log

/**
 * AudioPlayerManager: Manages audio playback and RootlessJamesDSP integration.
 * 
 * Coordinates between AudioPlayer (native engine) and AudioEffectBroadcaster
 * to ensure proper AudioEffect session binding for RootlessJamesDSP.
 */
class AudioPlayerManager(private val context: Context) {

    companion object {
        private const val TAG = "AudioPlayerManager"
    }

    private val audioPlayer = AudioPlayer()
    private val audioEffectBroadcaster = AudioEffectBroadcaster(context)
    private var currentSessionId: Int = 0

    /**
     * Initialize audio player and setup audio effect session.
     * 
     * @return true if initialization successful
     */
    fun initialize(): Boolean {
        return try {
            currentSessionId = audioPlayer.initialize()
            
            if (currentSessionId > 0) {
                Log.i(TAG, "Audio player initialized with session ID: $currentSessionId")
                true
            } else {
                Log.e(TAG, "Failed to initialize audio player")
                false
            }
        } catch (e: Exception) {
            Log.e(TAG, "Exception during initialization", e)
            false
        }
    }

    /**
     * Play an audio file and broadcast AudioEffect session opening.
     * 
     * @param filePath Path to the audio file
     * @return true if playback started successfully
     */
    fun playFile(filePath: String): Boolean {
        return try {
            if (currentSessionId <= 0) {
                Log.e(TAG, "Audio player not initialized")
                return false
            }

            // Start playback
            val playbackStarted = audioPlayer.playFile(filePath)
            
            if (playbackStarted) {
                // Broadcast AudioEffect session opening for RootlessJamesDSP
                audioEffectBroadcaster.broadcastOpenAudioEffectSession(
                    sessionId = currentSessionId,
                    contentType = android.media.audiofx.AudioEffect.CONTENT_TYPE_MUSIC
                )
                Log.i(TAG, "Playback started and AudioEffect session broadcasted")
            } else {
                Log.e(TAG, "Failed to start playback")
            }
            
            playbackStarted
        } catch (e: Exception) {
            Log.e(TAG, "Exception during playFile", e)
            false
        }
    }

    /**
     * Pause playback.
     */
    fun pause() {
        try {
            audioPlayer.pause()
            Log.d(TAG, "Playback paused")
        } catch (e: Exception) {
            Log.e(TAG, "Exception during pause", e)
        }
    }

    /**
     * Resume playback.
     */
    fun resume() {
        try {
            audioPlayer.resume()
            Log.d(TAG, "Playback resumed")
        } catch (e: Exception) {
            Log.e(TAG, "Exception during resume", e)
        }
    }

    /**
     * Stop playback and broadcast AudioEffect session closing.
     */
    fun stop() {
        try {
            audioPlayer.stop()
            
            // Broadcast AudioEffect session closing
            if (currentSessionId > 0) {
                audioEffectBroadcaster.broadcastCloseAudioEffectSession(currentSessionId)
                Log.i(TAG, "Playback stopped and AudioEffect session closed")
            }
        } catch (e: Exception) {
            Log.e(TAG, "Exception during stop", e)
        }
    }

    /**
     * Seek to a specific position.
     * 
     * @param positionMs Position in milliseconds
     */
    fun seekTo(positionMs: Long) {
        try {
            audioPlayer.seekTo(positionMs)
            Log.d(TAG, "Seeked to $positionMs ms")
        } catch (e: Exception) {
            Log.e(TAG, "Exception during seekTo", e)
        }
    }

    /**
     * Get current playback position.
     * 
     * @return Position in milliseconds
     */
    fun getCurrentPosition(): Long {
        return try {
            audioPlayer.getCurrentPosition()
        } catch (e: Exception) {
            Log.e(TAG, "Exception getting current position", e)
            0
        }
    }

    /**
     * Get total duration.
     * 
     * @return Duration in milliseconds
     */
    fun getDuration(): Long {
        return try {
            audioPlayer.getDuration()
        } catch (e: Exception) {
            Log.e(TAG, "Exception getting duration", e)
            0
        }
    }

    /**
     * Check if audio is playing.
     * 
     * @return true if playing
     */
    fun isPlaying(): Boolean {
        return try {
            audioPlayer.isPlaying()
        } catch (e: Exception) {
            Log.e(TAG, "Exception checking playback state", e)
            false
        }
    }

    /**
     * Get audio session ID.
     * 
     * @return Session ID for RootlessJamesDSP binding
     */
    fun getSessionId(): Int {
        return currentSessionId
    }

    /**
     * Get stream configuration info (for debugging).
     * 
     * @return Stream info string
     */
    fun getStreamInfo(): String {
        return try {
            audioPlayer.getStreamInfo()
        } catch (e: Exception) {
            Log.e(TAG, "Exception getting stream info", e)
            "Error retrieving stream info"
        }
    }

    /**
     * Release all resources.
     * Should be called when done with audio playback.
     */
    fun release() {
        try {
            // Stop playback and close AudioEffect session
            if (audioPlayer.isPlaying()) {
                stop()
            }
            
            // Release audio player
            audioPlayer.release()
            currentSessionId = 0
            
            Log.i(TAG, "Audio player manager released")
        } catch (e: Exception) {
            Log.e(TAG, "Exception during release", e)
        }
    }
}
