import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../styles"

/**
 * Individual subsystem module displayed on the system canvas
 * Shows health status, key metrics, and allows interaction
 * Uses individual properties for efficient model binding
 */
Rectangle {
    id: module
    
    // Individual properties for efficient model binding
    property string subsystemId: ""
    property string subsystemName: ""
    property string subsystemType: ""
    property string subsystemHealthState: "UNKNOWN"
    property real subsystemHealthScore: 100
    property int subsystemFaultCount: 0
    
    signal clicked()
    signal removeRequested()
    
    width: RadarTheme.moduleWidth
    height: RadarTheme.moduleHeight
    radius: RadarTheme.radiusMedium
    
    color: mouseArea.containsMouse ? RadarColors.surfaceHover : RadarColors.surface
    border.color: getHealthBorderColor()
    border.width: 2
    
    // Glow effect for health state
    Rectangle {
        anchors.fill: parent
        anchors.margins: -4
        radius: parent.radius + 4
        color: "transparent"
        border.color: RadarColors.getHealthGlowColor(module.subsystemHealthState)
        border.width: 8
        opacity: 0.5
        visible: module.subsystemHealthState !== "OK"
        
        SequentialAnimation on opacity {
            running: module.subsystemHealthState === "FAIL"
            loops: Animation.Infinite
            NumberAnimation { to: 0.2; duration: 800 }
            NumberAnimation { to: 0.6; duration: 800 }
        }
    }
    
    Behavior on color {
        ColorAnimation { duration: RadarTheme.animationFast }
    }
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        
        onClicked: module.clicked()
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: RadarTheme.spacingMedium
        spacing: RadarTheme.spacingSmall
        
        // Header row
        RowLayout {
            Layout.fillWidth: true
            spacing: RadarTheme.spacingSmall
            
            // Health indicator
            Rectangle {
                width: RadarTheme.healthIndicatorMedium
                height: RadarTheme.healthIndicatorMedium
                radius: width / 2
                color: RadarColors.getHealthColor(module.subsystemHealthState)
                
                SequentialAnimation on scale {
                    running: module.subsystemHealthState === "FAIL"
                    loops: Animation.Infinite
                    NumberAnimation { to: 1.3; duration: 400 }
                    NumberAnimation { to: 1.0; duration: 400 }
                }
            }
            
            // Name
            Text {
                Layout.fillWidth: true
                text: module.subsystemName
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeMedium
                font.weight: Font.Medium
                color: RadarColors.textPrimary
                elide: Text.ElideRight
            }
            
            // Close button
            Rectangle {
                width: 20
                height: 20
                radius: 10
                color: closeMouseArea.containsMouse ? RadarColors.healthFail : "transparent"
                visible: mouseArea.containsMouse
                
                Text {
                    anchors.centerIn: parent
                    text: "×"
                    font.pixelSize: 14
                    font.bold: true
                    color: closeMouseArea.containsMouse ? "white" : RadarColors.textTertiary
                }
                
                MouseArea {
                    id: closeMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: module.removeRequested()
                }
            }
        }
        
        // Type label
        Text {
            text: module.subsystemType
            font.family: RadarTheme.fontFamily
            font.pixelSize: RadarTheme.fontSizeXSmall
            color: RadarColors.textTertiary
        }
        
        // Health score bar
        Rectangle {
            Layout.fillWidth: true
            height: 4
            radius: 2
            color: RadarColors.backgroundDark
            
            Rectangle {
                width: parent.width * (module.subsystemHealthScore / 100)
                height: parent.height
                radius: 2
                color: RadarColors.getHealthColor(module.subsystemHealthState)
                
                Behavior on width {
                    NumberAnimation { duration: RadarTheme.animationMedium }
                }
            }
        }
        
        // Key metrics
        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 2
            rowSpacing: 4
            columnSpacing: RadarTheme.spacingSmall
            
            // Health score
            MetricItem {
                label: "Health"
                value: module.subsystemHealthScore.toFixed(0) + "%"
                color: RadarColors.getHealthColor(module.subsystemHealthState)
            }
            
            // Fault count
            MetricItem {
                label: "Faults"
                value: module.subsystemFaultCount.toString()
                color: module.subsystemFaultCount > 0 ? RadarColors.healthFail : RadarColors.textPrimary
            }
            
            // Status (dynamic based on subsystem type)
            MetricItem {
                label: "Status"
                value: module.subsystemHealthState
                color: RadarColors.getHealthColor(module.subsystemHealthState)
            }
            
            // ID
            MetricItem {
                label: "ID"
                value: module.subsystemId
                color: RadarColors.textSecondary
            }
        }
    }
    
    function getHealthBorderColor() {
        if (mouseArea.containsMouse) {
            return RadarColors.accent
        }
        return RadarColors.getHealthColor(module.subsystemHealthState)
    }
    
    // Metric item component
    component MetricItem: Column {
        property string label
        property string value
        property color color: RadarColors.textPrimary
        
        Layout.fillWidth: true
        spacing: 0
        
        Text {
            text: label
            font.family: RadarTheme.fontFamily
            font.pixelSize: RadarTheme.fontSizeXSmall
            color: RadarColors.textTertiary
        }
        
        Text {
            text: value
            font.family: RadarTheme.fontFamilyMono
            font.pixelSize: RadarTheme.fontSizeSmall
            font.weight: Font.Medium
            color: parent.color
        }
    }
}
