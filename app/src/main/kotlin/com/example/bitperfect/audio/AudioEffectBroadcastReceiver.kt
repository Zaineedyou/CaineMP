package com.example.bitperfect.audio

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.media.audiofx.AudioEffect
import android.util.Log

/**
 * AudioEffectBroadcastReceiver: Receives AudioEffect control intents.
 * 
 * This receiver listens for audio effect related broadcasts,
 * allowing other apps to control audio effects for this application.
 */
class AudioEffectBroadcastReceiver : BroadcastReceiver() {

    companion object {
        private const val TAG = "AudioEffectReceiver"
    }

    override fun onReceive(context: Context?, intent: Intent?) {
        if (context == null || intent == null) return

        when (intent.action) {
            AudioEffect.ACTION_OPEN_AUDIO_EFFECT_CONTROL_SESSION -> {
                handleOpenAudioEffectSession(intent)
            }
            AudioEffect.ACTION_CLOSE_AUDIO_EFFECT_CONTROL_SESSION -> {
                handleCloseAudioEffectSession(intent)
            }
            else -> {
                Log.w(TAG, "Unknown action: ${intent.action}")
            }
        }
    }

    private fun handleOpenAudioEffectSession(intent: Intent) {
        val sessionId = intent.getIntExtra(AudioEffect.EXTRA_AUDIO_SESSION, -1)
        val packageName = intent.getStringExtra(AudioEffect.EXTRA_PACKAGE_NAME)
        val contentType = intent.getIntExtra(AudioEffect.EXTRA_CONTENT_TYPE, -1)

        Log.i(TAG, "Received OPEN_AUDIO_EFFECT_CONTROL_SESSION: " +
            "sessionId=$sessionId, packageName=$packageName, contentType=$contentType")

        // TODO: Handle audio effect session opening if needed
    }

    private fun handleCloseAudioEffectSession(intent: Intent) {
        val sessionId = intent.getIntExtra(AudioEffect.EXTRA_AUDIO_SESSION, -1)
        val packageName = intent.getStringExtra(AudioEffect.EXTRA_PACKAGE_NAME)

        Log.i(TAG, "Received CLOSE_AUDIO_EFFECT_CONTROL_SESSION: " +
            "sessionId=$sessionId, packageName=$packageName")

        // TODO: Handle audio effect session closing if needed
    }
}
