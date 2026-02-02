pragma Singleton
import QtQuick 2.15

/**
 * Radar-style color palette for defence-grade UI
 * Dark theme with high contrast for command center environments
 */
QtObject {
    // Primary background colors
    readonly property color background: "#0a0e14"
    readonly property color backgroundLight: "#141a24"
    readonly property color backgroundMedium: "#1a2233"
    readonly property color backgroundDark: "#050810"
    
    // Surface colors
    readonly property color surface: "#1c2433"
    readonly property color surfaceLight: "#243040"
    readonly property color surfaceHover: "#2a3850"
    readonly property color surfaceBorder: "#2d3a4f"
    
    // Accent colors
    readonly property color accent: "#00d4ff"
    readonly property color accentDark: "#0099cc"
    readonly property color accentGlow: "#00d4ff40"
    
    // Health status colors
    readonly property color healthOk: "#00ff88"
    readonly property color healthOkGlow: "#00ff8840"
    readonly property color healthDegraded: "#ffaa00"
    readonly property color healthDegradedGlow: "#ffaa0040"
    readonly property color healthFail: "#ff3355"
    readonly property color healthFailGlow: "#ff335540"
    readonly property color healthUnknown: "#666688"
    
    // Telemetry colors (for different parameters)
    readonly property color telemetryPrimary: "#00d4ff"
    readonly property color telemetrySecondary: "#7b68ee"
    readonly property color telemetryTertiary: "#ff6b9d"
    readonly property color telemetryQuaternary: "#ffd93d"
    
    // Text colors
    readonly property color textPrimary: "#e8eaed"
    readonly property color textSecondary: "#9aa0a6"
    readonly property color textTertiary: "#666680"
    readonly property color textHighlight: "#00d4ff"
    
    // Border colors
    readonly property color border: "#2d3a4f"
    readonly property color borderLight: "#3d4a5f"
    readonly property color borderActive: "#00d4ff"
    
    // Chart colors
    readonly property var chartColors: [
        "#00d4ff",
        "#00ff88",
        "#ffaa00",
        "#ff6b9d",
        "#7b68ee",
        "#ffd93d",
        "#00ffcc",
        "#ff5533"
    ]
    
    // Severity colors
    readonly property color severityInfo: "#00d4ff"
    readonly property color severityWarning: "#ffaa00"
    readonly property color severityCritical: "#ff6633"
    readonly property color severityFatal: "#ff3355"
    
    // Gradients
    readonly property var healthOkGradient: Gradient {
        GradientStop { position: 0.0; color: "#00ff88" }
        GradientStop { position: 1.0; color: "#00cc66" }
    }
    
    readonly property var healthDegradedGradient: Gradient {
        GradientStop { position: 0.0; color: "#ffaa00" }
        GradientStop { position: 1.0; color: "#cc8800" }
    }
    
    readonly property var healthFailGradient: Gradient {
        GradientStop { position: 0.0; color: "#ff3355" }
        GradientStop { position: 1.0; color: "#cc2244" }
    }
    
    // Helper functions
    function getHealthColor(state) {
        switch(state) {
            case "OK": return healthOk
            case "DEGRADED": return healthDegraded
            case "FAIL": return healthFail
            default: return healthUnknown
        }
    }
    
    function getHealthGlowColor(state) {
        switch(state) {
            case "OK": return healthOkGlow
            case "DEGRADED": return healthDegradedGlow
            case "FAIL": return healthFailGlow
            default: return "transparent"
        }
    }
    
    function getSeverityColor(severity) {
        switch(severity) {
            case "INFO": return severityInfo
            case "WARNING": return severityWarning
            case "CRITICAL": return severityCritical
            case "FATAL": return severityFatal
            default: return severityInfo
        }
    }
}
