# ProGuard rules for BitPerfectPlayer

# Keep native methods (JNI)
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep Oboe classes
-keep class com.google.oboe.** { *; }
-dontwarn com.google.oboe.**

# Keep Compose classes
-keep class androidx.compose.** { *; }
-dontwarn androidx.compose.**

# Keep Android framework classes
-keep class android.** { *; }
-dontwarn android.**

# Keep application classes
-keep class com.example.bitperfect.** { *; }

# Keep audio-related classes
-keep class android.media.** { *; }
-keep class android.media.audiofx.** { *; }

# Preserve line numbers for debugging
-keepattributes SourceFile,LineNumberTable
-renamesourcefileattribute SourceFile
