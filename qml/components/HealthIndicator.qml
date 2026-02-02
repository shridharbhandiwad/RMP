import QtQuick 2.15
import "../styles"

/**
 * Visual health indicator with animated states
 */
Rectangle {
    id: indicator
    
    property string healthState: "UNKNOWN"
    property bool animated: true
    property bool showGlow: true
    
    width: RadarTheme.healthIndicatorMedium
    height: RadarTheme.healthIndicatorMedium
    radius: width / 2
    color: RadarColors.getHealthColor(healthState)
    
    // Glow effect
    Rectangle {
        anchors.centerIn: parent
        width: parent.width + 8
        height: parent.height + 8
        radius: width / 2
        color: "transparent"
        border.color: RadarColors.getHealthGlowColor(healthState)
        border.width: 4
        visible: showGlow && healthState !== "OK"
        opacity: glowOpacity
        
        property real glowOpacity: 0.5
        
        SequentialAnimation on glowOpacity {
            running: animated && healthState === "FAIL"
            loops: Animation.Infinite
            NumberAnimation { to: 0.2; duration: 600 }
            NumberAnimation { to: 0.8; duration: 600 }
        }
    }
    
    // Pulse animation for critical state
    SequentialAnimation on scale {
        running: animated && healthState === "FAIL"
        loops: Animation.Infinite
        NumberAnimation { to: 1.2; duration: 400 }
        NumberAnimation { to: 1.0; duration: 400 }
    }
}
