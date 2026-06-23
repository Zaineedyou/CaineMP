package com.example.bitperfect

import android.Manifest
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.ui.Modifier
import androidx.core.content.ContextCompat
import com.example.bitperfect.ui.screens.PlayerScreen
import com.example.bitperfect.ui.theme.BitPerfectPlayerTheme
import android.util.Log

/**
 * MainActivity: Entry point for BitPerfect Player application
 * 
 * Handles:
 * - Runtime permission requests (READ_MEDIA_AUDIO, RECORD_AUDIO)
 * - Jetpack Compose UI initialization
 * - Permission state management
 */
class MainActivity : ComponentActivity() {
    
    private var permissionsGranted = false
    
    // Runtime permission request launcher
    private val requestPermissionsLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        val allGranted = permissions.values.all { it }
        permissionsGranted = allGranted
        
        if (allGranted) {
            Log.i(TAG, "All permissions granted")
        } else {
            Log.w(TAG, "Some permissions denied")
            // Permissions denied - UI will show warning
        }
        
        // Refresh UI after permission result
        updateUI()
    }
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        // Check and request permissions
        checkAndRequestPermissions()
        
        updateUI()
    }
    
    /**
     * Check current permission status and request if needed.
     */
    private fun checkAndRequestPermissions() {
        val requiredPermissions = getRequiredPermissions()
        val missingPermissions = requiredPermissions.filter { permission ->
            ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED
        }
        
        if (missingPermissions.isNotEmpty()) {
            Log.i(TAG, "Requesting permissions: $missingPermissions")
            requestPermissionsLauncher.launch(missingPermissions.toTypedArray())
        } else {
            Log.i(TAG, "All permissions already granted")
            permissionsGranted = true
        }
    }
    
    /**
     * Get list of required permissions based on Android version.
     * 
     * Android 13+ (API 33+): Use READ_MEDIA_AUDIO
     * Android <13: Use READ_EXTERNAL_STORAGE
     */
    private fun getRequiredPermissions(): List<String> {
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            // Android 13+ (API 33+)
            listOf(
                Manifest.permission.READ_MEDIA_AUDIO,
                Manifest.permission.RECORD_AUDIO
            )
        } else {
            // Android <13
            listOf(
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.RECORD_AUDIO
            )
        }
    }
    
    /**
     * Update UI with current permission state.
     */
    private fun updateUI() {
        setContent {
            BitPerfectPlayerTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    PlayerScreen(
                        context = this@MainActivity,
                        permissionsGranted = permissionsGranted
                    )
                }
            }
        }
    }
    
    companion object {
        private const val TAG = "MainActivity"
    }
}
