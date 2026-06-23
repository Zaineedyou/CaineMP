package com.example.bitperfect.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material.icons.filled.Pause
import androidx.compose.material.icons.filled.Stop
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import android.content.Context
import android.media.MediaPlayer
import android.os.Environment
import com.example.bitperfect.audio.AudioPlayerManager
import java.io.File

/**
 * PlayerScreen: Main UI for BitPerfect Player
 * 
 * Features:
 * - Local audio file browser
 * - Play/Pause/Stop controls
 * - Progress seekbar
 * - Stream info display
 * - Real-time playback state
 */
@Composable
fun PlayerScreen(context: Context, permissionsGranted: Boolean = false) {
    val audioPlayerManager = remember { AudioPlayerManager(context) }
    
    // Initialize audio player
    LaunchedEffect(Unit) {
        audioPlayerManager.initialize()
    }
    
    var isPlaying by remember { mutableStateOf(false) }
    var currentPosition by remember { mutableStateOf(0L) }
    var duration by remember { mutableStateOf(0L) }
    var streamInfo by remember { mutableStateOf("Not initialized") }
    var selectedFile by remember { mutableStateOf<File?>(null) }
    var audioFiles by remember { mutableStateOf<List<File>>(emptyList()) }
    
    // Load audio files from Music directory (only if permissions granted)
    LaunchedEffect(permissionsGranted) {
        if (permissionsGranted) {
            audioFiles = getAudioFiles(context)
        } else {
            audioFiles = emptyList()
        }
    }
    
    // Update playback state periodically
    LaunchedEffect(isPlaying) {
        while (isPlaying) {
            currentPosition = audioPlayerManager.getCurrentPosition()
            duration = audioPlayerManager.getDuration()
            streamInfo = audioPlayerManager.getStreamInfo()
            kotlinx.coroutines.delay(100)
        }
    }
    
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
            .padding(16.dp),
        verticalArrangement = Arrangement.SpaceBetween
    ) {
        // Header
        Text(
            text = "BitPerfect Player",
            fontSize = 28.sp,
            fontWeight = FontWeight.Bold,
            color = MaterialTheme.colorScheme.primary,
            modifier = Modifier.padding(bottom = 16.dp)
        )
        
        // Permission warning (if not granted)
        if (!permissionsGranted) {
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(bottom = 16.dp),
                colors = CardDefaults.cardColors(
                    containerColor = MaterialTheme.colorScheme.errorContainer
                )
            ) {
                Text(
                    text = "⚠️ Permission Required\nPlease grant audio and file access permissions to use this app.",
                    modifier = Modifier.padding(12.dp),
                    color = MaterialTheme.colorScheme.onErrorContainer,
                    fontSize = 12.sp
                )
            }
        }
        
        // File List
        Text(
            text = "Audio Files",
            fontSize = 16.sp,
            fontWeight = FontWeight.SemiBold,
            color = MaterialTheme.colorScheme.onBackground,
            modifier = Modifier.padding(bottom = 8.dp)
        )
        
        LazyColumn(
            modifier = Modifier
                .fillMaxWidth()
                .weight(1f)
                .background(
                    color = MaterialTheme.colorScheme.surface,
                    shape = RoundedCornerShape(8.dp)
                )
                .padding(8.dp)
        ) {
            items(audioFiles) { file ->
                FileListItem(
                    file = file,
                    isSelected = selectedFile == file,
                    onClick = { selectedFile = file }
                )
            }
            
            if (audioFiles.isEmpty()) {
                item {
                    Text(
                        text = "No audio files found in Music folder",
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(16.dp),
                        color = MaterialTheme.colorScheme.onSurface.copy(alpha = 0.6f)
                    )
                }
            }
        }
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Stream Info
        StreamInfoCard(streamInfo)
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Progress Bar
        if (duration > 0) {
            Column(modifier = Modifier.fillMaxWidth()) {
                Slider(
                    value = if (duration > 0) currentPosition.toFloat() else 0f,
                    onValueChange = { newPosition ->
                        audioPlayerManager.seekTo(newPosition.toLong())
                    },
                    valueRange = 0f..duration.toFloat(),
                    modifier = Modifier.fillMaxWidth()
                )
                
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Text(
                        text = formatTime(currentPosition),
                        fontSize = 12.sp,
                        color = MaterialTheme.colorScheme.onBackground
                    )
                    Text(
                        text = formatTime(duration),
                        fontSize = 12.sp,
                        color = MaterialTheme.colorScheme.onBackground
                    )
                }
            }
        }
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Control Buttons
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .height(60.dp),
            horizontalArrangement = Arrangement.SpaceEvenly,
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Play Button
            FloatingActionButton(
                onClick = {
                    if (selectedFile != null) {
                        val success = audioPlayerManager.playFile(selectedFile!!.absolutePath)
                        if (success) {
                            isPlaying = true
                        }
                    }
                },
                modifier = Modifier.size(56.dp),
                containerColor = MaterialTheme.colorScheme.primary
            ) {
                Icon(
                    imageVector = Icons.Default.PlayArrow,
                    contentDescription = "Play",
                    tint = Color.White
                )
            }
            
            // Pause Button
            FloatingActionButton(
                onClick = {
                    audioPlayerManager.pause()
                    isPlaying = false
                },
                modifier = Modifier.size(56.dp),
                containerColor = MaterialTheme.colorScheme.secondary
            ) {
                Icon(
                    imageVector = Icons.Filled.Pause,
                    contentDescription = "Pause",
                    tint = Color.White
                )
            }
            
            // Stop Button
            FloatingActionButton(
                onClick = {
                    audioPlayerManager.stop()
                    isPlaying = false
                    currentPosition = 0L
                },
                modifier = Modifier.size(56.dp),
                containerColor = MaterialTheme.colorScheme.tertiary
            ) {
                Icon(
                    imageVector = Icons.Filled.Stop,
                    contentDescription = "Stop",
                    tint = Color.White
                )
            }
        }
        
        Spacer(modifier = Modifier.height(8.dp))
        
        // Status Text
        Text(
            text = if (isPlaying) "▶ Playing" else if (selectedFile != null) "⏸ Ready" else "⏹ Stopped",
            fontSize = 14.sp,
            fontWeight = FontWeight.SemiBold,
            color = if (isPlaying) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.onBackground,
            modifier = Modifier.align(Alignment.CenterHorizontally)
        )
    }
}

@Composable
private fun FileListItem(
    file: File,
    isSelected: Boolean,
    onClick: () -> Unit
) {
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp)
            .clickable(onClick = onClick),
        color = if (isSelected) MaterialTheme.colorScheme.primaryContainer else Color.Transparent,
        shape = RoundedCornerShape(4.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = file.nameWithoutExtension,
                    fontSize = 14.sp,
                    fontWeight = FontWeight.Medium,
                    color = MaterialTheme.colorScheme.onSurface,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
                Text(
                    text = file.extension.uppercase(),
                    fontSize = 12.sp,
                    color = MaterialTheme.colorScheme.onSurface.copy(alpha = 0.6f)
                )
            }
        }
    }
}

@Composable
private fun StreamInfoCard(streamInfo: String) {
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .background(
                color = MaterialTheme.colorScheme.surface,
                shape = RoundedCornerShape(8.dp)
            ),
        shape = RoundedCornerShape(8.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp)
        ) {
            Text(
                text = "Stream Info",
                fontSize = 12.sp,
                fontWeight = FontWeight.SemiBold,
                color = MaterialTheme.colorScheme.onSurface
            )
            Text(
                text = streamInfo,
                fontSize = 10.sp,
                fontFamily = FontFamily.Monospace,
                color = MaterialTheme.colorScheme.onSurface.copy(alpha = 0.7f),
                maxLines = 3,
                overflow = TextOverflow.Ellipsis,
                modifier = Modifier.padding(top = 4.dp)
            )
        }
    }
}

private fun formatTime(ms: Long): String {
    val totalSeconds = ms / 1000
    val minutes = totalSeconds / 60
    val seconds = totalSeconds % 60
    return String.format("%02d:%02d", minutes, seconds)
}

private fun getAudioFiles(context: Context): List<File> {
    val musicDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MUSIC)
    
    return if (musicDir.exists() && musicDir.isDirectory) {
        musicDir.listFiles { file ->
            file.isFile && file.extension.lowercase() in listOf("mp3", "wav", "flac", "m4a", "ogg")
        }?.sortedBy { it.name } ?: emptyList()
    } else {
        emptyList()
    }
}
