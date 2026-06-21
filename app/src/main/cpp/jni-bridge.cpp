#include <jni.h>
#include "audio-engine.h"
#include <android/log.h>
#include <inttypes.h>

#define TAG "JNIBridge"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// Global AudioEngine instance (singleton pattern)
static AudioEngine* g_audioEngine = nullptr;

extern "C" {

/**
 * Initialize the audio engine.
 * 
 * Returns the audio session ID for AudioEffect binding.
 * Must be called before any playback operations.
 */
JNIEXPORT jint JNICALL
Java_com_example_bitperfect_audio_AudioPlayer_nativeInit(
    JNIEnv* env,
    jobject thiz) {
    
    LOGI("nativeInit called");
    
    try {
        if (g_audioEngine == nullptr) {
            g_audioEngine = new AudioEngine();
        }
        
        int32_t sessionId = g_audioEngine->initialize();
        LOGI("Audio engine initialized with session ID: %d", sessionId);
        
        return sessionId;
    } catch (const std::exception& e) {
        LOGE("Exception in nativeInit: %s", e.what());
        return 0;
    }
}

/**
 * Play an audio file.
 * 
 * @param filePath Path to the audio file
 * @return true if playback started successfully
 */
JNIEXPORT jboolean JNICALL
Java_com_example_bitperfect_audio_AudioPlayer_nativePlayFile(
    JNIEnv* env,
    jobject thiz,
    jstring filePath) {
    
    if (g_audioEngine == nullptr) {
        LOGE("Audio engine not initialized");
        return false;
    }
    
    try {
        const char* path = env->GetStringUTFChars(filePath, nullptr);
        if (path == nullptr) {
            LOGE("Failed to get file path from JNI");
            return false;
        }
        
        LOGI("nativePlayFile: %s", path);
        
        bool result = g_audioEngine->playFile(path);
        
        env->ReleaseStringUTFChars(filePath, path);
        
        return result;
    } catch (const std::exception& e) {
        LOGE("Exception in nativePlayFile: %s", e.what());
        return false;
    }
}

/**
 * Pause playback.
 */
JNIEXPORT void JNICALL
Java_com_example_bitperfect_audio_AudioPlayer_nativePause(
    JNIEnv* env,
    jobject thiz) {
    
    LOGI("nativePause called");
    
    try {
        if (g_audioEngine != nullptr) {
            g_audioEngine->pause();
        }
    } catch (const std::exception& e) {
        LOGE("Exception in nativePause: %s", e.what());
    }
}

/**
 * Resume playback.
 */
JNIEXPORT void JNICALL
Java_com_example_bitperfect_audio_AudioPlayer_nativeResume(
    JNIEnv* env,
    jobject thiz) {
    
    LOGI("nativeResume called");
    
    try {
        if (g_audioEngine != nullptr) {
            g_audioEngine->resume();
        }
    } catch (const std::exception& e) {
        LOGE("Exception in nativeResume: %s", e.what());
    }
}

/**
 * Stop playback.
 */
JNIEXPORT void JNICALL
Java_com_example_bitperfect_audio_AudioPlayer_nativeStop(
    JNIEnv* env,
    jobject thiz) {
    
    LOGI("nativeStop called");
    
    try {
        if (g_audioEngine != nullptr) {
            g_audioEngine->stop();
        }
    } catch (const std::exception& e) {
        LOGE("Exception in nativeStop: %s", e.what());
    }
}

/**
 * Seek to a specific position.
 * 
 * @param positionMs Position in milliseconds
 */
JNIEXPORT void JNICALL
Java_com_example_bitperfect_audio_AudioPlayer_nativeSeekTo(
    JNIEnv* env,
    jobject thiz,
    jlong positionMs) {
    
    LOGI("nativeSeekTo: %" PRId64 " ms", positionMs);
    
    try {
        if (g_audioEngine != nullptr) {
            g_audioEngine->seekTo(positionMs);
        }
    } catch (const std::exception& e) {
        LOGE("Exception in nativeSeekTo: %s", e.what());
    }
}

/**
 * Get current playback position.
 * 
 * @return Position in milliseconds
 */
JNIEXPORT jlong JNICALL
Java_com_example_bitperfect_audio_AudioPlayer_nativeGetCurrentPosition(
    JNIEnv* env,
    jobject thiz) {
    
    try {
        if (g_audioEngine == nullptr) {
            return 0;
        }
        
        return g_audioEngine->getCurrentPosition();
    } catch (const std::exception& e) {
        LOGE("Exception in nativeGetCurrentPosition: %s", e.what());
        return 0;
    }
}

/**
 * Get total duration.
 * 
 * @return Duration in milliseconds
 */
JNIEXPORT jlong JNICALL
Java_com_example_bitperfect_audio_AudioPlayer_nativeGetDuration(
    JNIEnv* env,
    jobject thiz) {
    
    try {
        if (g_audioEngine == nullptr) {
            return 0;
        }
        
        return g_audioEngine->getDuration();
    } catch (const std::exception& e) {
        LOGE("Exception in nativeGetDuration: %s", e.what());
        return 0;
    }
}

/**
 * Check if audio is playing.
 * 
 * @return true if playing, false otherwise
 */
JNIEXPORT jboolean JNICALL
Java_com_example_bitperfect_audio_AudioPlayer_nativeIsPlaying(
    JNIEnv* env,
    jobject thiz) {
    
    try {
        if (g_audioEngine == nullptr) {
            return false;
        }
        
        return g_audioEngine->isPlaying();
    } catch (const std::exception& e) {
        LOGE("Exception in nativeIsPlaying: %s", e.what());
        return false;
    }
}

/**
 * Get audio session ID.
 * 
 * @return Session ID for AudioEffect binding
 */
JNIEXPORT jint JNICALL
Java_com_example_bitperfect_audio_AudioPlayer_nativeGetSessionId(
    JNIEnv* env,
    jobject thiz) {
    
    try {
        if (g_audioEngine == nullptr) {
            return 0;
        }
        
        return g_audioEngine->getSessionId();
    } catch (const std::exception& e) {
        LOGE("Exception in nativeGetSessionId: %s", e.what());
        return 0;
    }
}

/**
 * Get stream configuration info (for debugging).
 * 
 * @return Stream info string
 */
JNIEXPORT jstring JNICALL
Java_com_example_bitperfect_audio_AudioPlayer_nativeGetStreamInfo(
    JNIEnv* env,
    jobject thiz) {
    
    try {
        if (g_audioEngine == nullptr) {
            return env->NewStringUTF("Audio engine not initialized");
        }
        
        std::string info = g_audioEngine->getStreamInfo();
        return env->NewStringUTF(info.c_str());
    } catch (const std::exception& e) {
        LOGE("Exception in nativeGetStreamInfo: %s", e.what());
        return env->NewStringUTF("Error getting stream info");
    }
}

/**
 * Release audio engine resources.
 * 
 * Should be called when the player is no longer needed.
 */
JNIEXPORT void JNICALL
Java_com_example_bitperfect_audio_AudioPlayer_nativeRelease(
    JNIEnv* env,
    jobject thiz) {
    
    LOGI("nativeRelease called");
    
    try {
        if (g_audioEngine != nullptr) {
            delete g_audioEngine;
            g_audioEngine = nullptr;
        }
    } catch (const std::exception& e) {
        LOGE("Exception in nativeRelease: %s", e.what());
    }
}

} // extern "C"
