package com.example.bitperfect

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.ui.Modifier
import com.example.bitperfect.ui.screens.PlayerScreen
import com.example.bitperfect.ui.theme.BitPerfectPlayerTheme

/**
 * MainActivity: Entry point for BitPerfect Player application
 * 
 * Initializes Jetpack Compose and displays the player UI.
 */
class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            BitPerfectPlayerTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    PlayerScreen(context = this@MainActivity)
                }
            }
        }
    }
}
