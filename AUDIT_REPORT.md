# CaineMP Audio Engine - Audit Report & Fix Plan

**Date**: June 21, 2026  
**Project**: CaineMP (BitPerfect Music Player)  
**Status**: 3 Critical Bugs Identified

---

## Executive Summary

CaineMP has successfully built and deployed the APK with Jetpack Compose UI and basic AAudio support. However, 3 critical runtime issues prevent audio playback:

1. **Missing Runtime Permissions** - App crashes when accessing files without proper permission grants
2. **MP3 Format Not Supported** - Audio engine only supports WAV; MP3 files detected but fail to decode
3. **Audio Engine Initialization Failure** - Exclusive mode fails silently; no fallback to Shared mode; status never updates

---

## Bug #1: Missing Runtime Permissions

### Current State
- **File**: `MainActivity.kt` (lines 1-32)
- **Issue**: No runtime permission requests implemented
- **Manifest**: Permissions declared (`READ_EXTERNAL_STORAGE`, `READ_MEDIA_AUDIO`, `RECORD_AUDIO`) but NOT requested at runtime
- **Android Version**: App targets API 34 but doesn't use Android 13+ `READ_MEDIA_AUDIO` permission
- **Result**: `SecurityException` when `getAudioFiles()` tries to access `Environment.getExternalStoragePublicDirectory()`

### Root Cause
```kotlin
// PlayerScreen.kt line 57
LaunchedEffect(Unit) {
    audioFiles = getAudioFiles(context)  // ❌ No permission check before file access
}
```

### Impact
- App crashes on first launch when trying to scan Music folder
- Even if permissions granted manually via Settings, app doesn't detect it
- Play button never works because file list is empty

### Fix Required
1. Implement `ActivityResultContracts.RequestMultiplePermissions()` in `MainActivity`
2. Check permissions before `getAudioFiles()` call in `PlayerScreen`
3. Handle Android 13+ (API 33+) with `READ_MEDIA_AUDIO` instead of `READ_EXTERNAL_STORAGE`
4. Show permission request dialog on first launch

---

## Bug #2: MP3 Format Not Supported

### Current State
- **File**: `audio-engine.cpp` (lines 299-400)
- **Issue**: `decodeAudioFile()` only handles WAV format (PCM)
- **Line 344-348**: Rejects any non-PCM audio format
- **Result**: MP3 files detected by UI but fail silently during playback

### Root Cause
```cpp
// audio-engine.cpp line 344-348
if (fmtChunk.audioFormat != 1) {  // 1 = PCM only
    LOGE("Only PCM audio format supported (format=%d)", fmtChunk.audioFormat);
    file.close();
    return false;
}
```

### Impact
- UI shows MP3 files in list (user selects them)
- Play button appears to work (no error thrown)
- But audio engine fails to decode → no sound output
- Status shows "Not initialized" because playback fails silently

### Fix Required
1. Implement MP3 decoder using Android NDK `AMediaExtractor` + `AMediaCodec`
2. Detect MP3 format by file extension or MIME type
3. Extract PCM data from MP3 and feed to Oboe stream
4. Update `CMakeLists.txt` to link `mediandk` library
5. Handle MP3 metadata (sample rate, channels) dynamically

---

## Bug #3: Audio Engine Initialization Failure

### Current State
- **File**: `audio-engine.cpp` (lines 258-286)
- **Issue**: Exclusive mode fails → no fallback to Shared mode
- **Status**: Never updates from "Not initialized" to "Playing"
- **Result**: Play button doesn't work even for WAV files

### Root Cause
```cpp
// audio-engine.cpp line 264
builder.setSharingMode(SharingMode::Exclusive);  // ❌ Fails on many devices
Result result = builder.openStream(audioStream_);
if (result != Result::OK) {
    LOGE("Failed to open audio stream: %s", convertToText(result));
    return result;  // ❌ No fallback, just fails
}
```

### Impact
- Exclusive mode not available on all Android devices/hardware
- Stream creation fails silently
- `g_audioEngine->playFile()` returns false
- UI status remains "Not initialized"
- Play button appears unresponsive

### Secondary Issue: Status Never Updates
- **File**: `PlayerScreen.kt` (lines 51, 60-68)
- **Issue**: `streamInfo` initialized to "Not initialized" (line 51)
- **Update Logic**: Only updates inside `while (isPlaying)` loop (lines 60-68)
- **Problem**: If initialization succeeds but playback hasn't started, status never shows "Ready"

### Fix Required
1. Implement fallback: Try Exclusive mode → if fails, try Shared mode
2. Add status string management in C++ (return "Ready", "Playing", "Error")
3. Update JNI `nativeGetStreamInfo()` to return status
4. Update `PlayerScreen` to show status after successful init (not just during playback)

---

## Implementation Plan

### Phase 1: Runtime Permissions (Kotlin)
- Add `ActivityResultContracts.RequestMultiplePermissions()` in `MainActivity`
- Check `READ_MEDIA_AUDIO` (API 33+) or `READ_EXTERNAL_STORAGE` (API <33)
- Request `RECORD_AUDIO` permission
- Block file access until permissions granted
- **Files to modify**: `MainActivity.kt`, `PlayerScreen.kt`

### Phase 2: MP3 Decoder (C++)
- Detect MP3 by file extension or MIME type
- Use `AMediaExtractor` to parse MP3 file
- Use `AMediaCodec` to decode MP3 → PCM
- Extract sample rate, channels, duration from MP3 metadata
- Feed PCM data to Oboe stream
- **Files to modify**: `audio-engine.cpp`, `CMakeLists.txt`

### Phase 3: Audio Engine Initialization (C++)
- Add fallback logic: Exclusive → Shared mode
- Add status string management ("Not initialized", "Ready", "Playing", "Error")
- Update JNI to return status
- Update `PlayerScreen` to show status after init
- **Files to modify**: `audio-engine.h`, `audio-engine.cpp`, `jni-bridge.cpp`, `PlayerScreen.kt`

---

## Testing Strategy

1. **Permission Flow**
   - Install APK fresh
   - Verify permission dialog appears
   - Grant permissions
   - Verify file list populates

2. **WAV Playback**
   - Place WAV file in Music folder
   - Select and play
   - Verify audio output

3. **MP3 Playback**
   - Place MP3 file in Music folder
   - Select and play
   - Verify audio output (same quality as WAV)

4. **Fallback Mode**
   - Test on device without Exclusive mode support
   - Verify Shared mode fallback works
   - Verify status shows "Ready" or "Playing"

---

## Risk Assessment

| Bug | Severity | Complexity | Risk |
|-----|----------|-----------|------|
| Runtime Permissions | HIGH | LOW | Low (standard Android pattern) |
| MP3 Decoder | HIGH | HIGH | Medium (NDK media APIs) |
| Init Fallback | HIGH | MEDIUM | Low (straightforward logic) |

---

## Estimated Timeline

- **Phase 1 (Permissions)**: 30 minutes
- **Phase 2 (MP3 Decoder)**: 2 hours
- **Phase 3 (Init Fallback)**: 45 minutes
- **Testing**: 1 hour
- **Total**: ~4.5 hours

---

## Deliverables

1. ✅ This audit report
2. ⏳ Updated `MainActivity.kt` with permission logic
3. ⏳ Updated `audio-engine.cpp` with MP3 decoder
4. ⏳ Updated `audio-engine.cpp` with init fallback
5. ⏳ Updated `CMakeLists.txt` with mediandk linkage
6. ⏳ Updated `PlayerScreen.kt` with permission checks
7. ⏳ Comprehensive documentation
8. ⏳ GitHub commit with all fixes

---

**Next Step**: Proceed to Phase 1 implementation (Runtime Permissions)
