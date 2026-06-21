plugins {
    id("com.android.application")
    kotlin("android")
}

android {
    namespace = "com.example.bitperfect"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.example.bitperfect"
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "1.0.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        // Enable audio playback capture for RootlessJamesDSP compatibility
        manifestPlaceholders["allowAudioPlaybackCapture"] = "true"

        // CMake configuration for native C++ compilation
        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
                cppFlags += "-fexceptions"
                cppFlags += "-frtti"
                arguments += "-DANDROID_STL=c++_shared"
                arguments += "-DANDROID_PLATFORM=android-24"
            }
        }

        // Enable Oboe prefab package
        packagingOptions {
            pickFirst("lib/arm64-v8a/libc++_shared.so")
            pickFirst("lib/armeabi-v7a/libc++_shared.so")
            pickFirst("lib/x86/libc++_shared.so")
            pickFirst("lib/x86_64/libc++_shared.so")
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
        debug {
            isDebuggable = true
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    kotlinOptions {
        jvmTarget = "11"
    }

    buildFeatures {
        prefab = true
        compose = true
        viewBinding = true
    }

    composeOptions {
        kotlinCompilerExtensionVersion = "1.5.8"
    }

    externalNativeBuild {
        cmake {
            path = file("CMakeLists.txt")
            version = "3.22.1"
        }
    }
}

dependencies {
    // Android Core & Lifecycle
    implementation("androidx.core:core-ktx:1.12.0")
    implementation("androidx.lifecycle:lifecycle-runtime-ktx:2.7.0")
    implementation("androidx.activity:activity-compose:1.8.1")

    // Jetpack Compose
    implementation("androidx.compose.ui:ui:1.6.4")
    implementation("androidx.compose.ui:ui-graphics:1.6.4")
    implementation("androidx.compose.ui:ui-tooling-preview:1.6.4")
    implementation("androidx.compose.material3:material3:1.2.0")
    debugImplementation("androidx.compose.ui:ui-tooling:1.6.4")
    debugImplementation("androidx.compose.ui:ui-test-manifest:1.6.4")
    implementation("com.google.android.material:material:1.11.0")

    // Oboe Audio Library (for AAudio/OpenSL ES)
    implementation("com.google.oboe:oboe:1.10.0")

    // Media & File Access
    implementation("androidx.media:media:1.7.0")
    implementation("androidx.documentfile:documentfile:1.0.1")

    // Coroutines
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.7.3")
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:1.7.3")

    // Testing
    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test.ext:junit:1.1.5")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.5.1")
    androidTestImplementation("androidx.compose.ui:ui-test-junit4:1.6.4")
}
