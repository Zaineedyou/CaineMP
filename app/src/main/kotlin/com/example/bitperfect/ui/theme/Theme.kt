package com.example.bitperfect.ui.theme

import androidx.compose.foundation.isSystemInDarkMode
import androidx.compose.material3.ColorScheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

// Light theme colors
private val LightColorScheme = lightColorScheme(
    primary = Color(0xFF006A4E),
    onPrimary = Color(0xFFFFFFFF),
    primaryContainer = Color(0xFF7FF8D6),
    onPrimaryContainer = Color(0xFF002019),
    secondary = Color(0xFF4F6358),
    onSecondary = Color(0xFFFFFFFF),
    secondaryContainer = Color(0xFFD1E8DB),
    onSecondaryContainer = Color(0xFF0D2018),
    tertiary = Color(0xFF386666),
    onTertiary = Color(0xFFFFFFFF),
    tertiaryContainer = Color(0xFFBBEBEB),
    onTertiaryContainer = Color(0xFF002020),
    error = Color(0xFFB3261E),
    onError = Color(0xFFFFFFFF),
    errorContainer = Color(0xFFF9DEDC),
    onErrorContainer = Color(0xFF410E0B),
    background = Color(0xFFFBFDF9),
    onBackground = Color(0xFF1A1C1A),
    surface = Color(0xFFFBFDF9),
    onSurface = Color(0xFF1A1C1A),
    surfaceVariant = Color(0xFFDCE4DE),
    onSurfaceVariant = Color(0xFF404943),
    outline = Color(0xFF707973),
    outlineVariant = Color(0xFFC0C9C2),
    scrim = Color(0xFF000000),
    inverseSurface = Color(0xFF2F312E),
    inverseOnSurface = Color(0xFFF1F5F0),
    inversePrimary = Color(0xFF5FDBBB),
)

// Dark theme colors
private val DarkColorScheme = darkColorScheme(
    primary = Color(0xFF5FDBBB),
    onPrimary = Color(0xFF003730),
    primaryContainer = Color(0xFF005141),
    onPrimaryContainer = Color(0xFF7FF8D6),
    secondary = Color(0xFFB5CCBF),
    onSecondary = Color(0xFF22342B),
    secondaryContainer = Color(0xFF384D42),
    onSecondaryContainer = Color(0xFFD1E8DB),
    tertiary = Color(0xFF9FCFCF),
    onTertiary = Color(0xFF003F3F),
    tertiaryContainer = Color(0xFF1F5555),
    onTertiaryContainer = Color(0xFFBBEBEB),
    error = Color(0xFFF2B8B5),
    onError = Color(0xFF601410),
    errorContainer = Color(0xFF8C1D18),
    onErrorContainer = Color(0xFFF9DEDC),
    background = Color(0xFF1A1C1A),
    onBackground = Color(0xFFE3E3E0),
    surface = Color(0xFF1A1C1A),
    onSurface = Color(0xFFE3E3E0),
    surfaceVariant = Color(0xFF404943),
    onSurfaceVariant = Color(0xFFC0C9C2),
    outline = Color(0xFF8A938C),
    outlineVariant = Color(0xFF404943),
    scrim = Color(0xFF000000),
    inverseSurface = Color(0xFFE3E3E0),
    inverseOnSurface = Color(0xFF2F312E),
    inversePrimary = Color(0xFF006A4E),
)

@Composable
fun BitPerfectPlayerTheme(
    darkTheme: Boolean = isSystemInDarkMode(),
    content: @Composable () -> Unit
) {
    val colorScheme = when {
        darkTheme -> DarkColorScheme
        else -> LightColorScheme
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}
