package com.example.bitperfect.audio

import android.content.Context
import android.content.Intent
import android.media.audiofx.AudioEffect
import android.util.Log

/**
 * AudioEffectBroadcaster: Handles broadcasting AudioEffect intents
 * for RootlessJamesDSP and other audio effect applications.
 */
class AudioEffectBroadcaster(private val context: Context) {

    companion object {
        private const val TAG = "AudioEffectBroadcaster"
    }

    /**
     * Broadcast ACTION_OPEN_AUDIO_EFFECT_CONTROL_SESSION intent.
     * This signals to RootlessJamesDSP and other effect apps that
     * a new audio session is opened and ready for effects.
     * 
     * @param sessionId Audio session ID from the audio engine
     * @param contentType Type of content (MUSIC, VOICE, MOVIE, etc.)
     */
    fun broadcastOpenAudioEffectSession(
        sessionId: Int,
        contentType: Int = AudioEffect.CONTENT_TYPE_MUSIC
    ) {
        try {
            val intent = Intent(AudioEffect.ACTION_OPEN_AUDIO_EFFECT_CONTROL_SESSION).apply {
                putExtra(AudioEffect.EXTRA_AUDIO_SESSION, sessionId)
                putExtra(AudioEffect.EXTRA_PACKAGE_NAME, context.packageName)
                putExtra(AudioEffect.EXTRA_CONTENT_TYPE, contentType)
            }
            
            context.sendBroadcast(intent)
            Log.i(TAG, "Broadcasted ACTION_OPEN_AUDIO_EFFECT_CONTROL_SESSION with sessionId=$sessionId")
        } catch (e: Exception) {
            Log.e(TAG, "Error broadcasting open audio effect session", e)
        }
    }

    /**
     * Broadcast ACTION_CLOSE_AUDIO_EFFECT_CONTROL_SESSION intent.
     * This signals to effect apps that the audio session is closed
     * and effects should no longer be applied.
     * 
     * @param sessionId Audio session ID from the audio engine
     */
    fun broadcastCloseAudioEffectSession(sessionId: Int) {
        try {
            val intent = Intent(AudioEffect.ACTION_CLOSE_AUDIO_EFFECT_CONTROL_SESSION).apply {
                putExtra(AudioEffect.EXTRA_AUDIO_SESSION, sessionId)
                putExtra(AudioEffect.EXTRA_PACKAGE_NAME, context.packageName)
            }
            
            context.sendBroadcast(intent)
            Log.i(TAG, "Broadcasted ACTION_CLOSE_AUDIO_EFFECT_CONTROL_SESSION with sessionId=$sessionId")
        } catch (e: Exception) {
            Log.e(TAG, "Error broadcasting close audio effect session", e)
        }
    }
}
