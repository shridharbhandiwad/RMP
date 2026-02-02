import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../styles"

/**
 * Left palette showing available subsystem modules
 * Supports drag-and-drop to system canvas
 */
Rectangle {
    id: palette
    color: RadarColors.backgroundLight
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            color: RadarColors.backgroundMedium
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: RadarTheme.spacingMedium
                anchors.rightMargin: RadarTheme.spacingMedium
                
                Text {
                    text: "SUBSYSTEM MODULES"
                    font.family: RadarTheme.fontFamily
                    font.pixelSize: RadarTheme.fontSizeSmall
                    font.weight: Font.Bold
                    font.letterSpacing: 1
                    color: RadarColors.textSecondary
                }
                
                Item { Layout.fillWidth: true }
                
                Text {
                    text: subsystemManager.totalSubsystemCount + " Available"
                    font.family: RadarTheme.fontFamily
                    font.pixelSize: RadarTheme.fontSizeSmall
                    color: RadarColors.textTertiary
                }
            }
            
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: RadarColors.border
            }
        }
        
        // Search box
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            Layout.margins: RadarTheme.spacingMedium
            color: RadarColors.surface
            radius: RadarTheme.radiusMedium
            border.color: searchField.activeFocus ? RadarColors.accent : RadarColors.border
            border.width: 1
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: RadarTheme.spacingMedium
                anchors.rightMargin: RadarTheme.spacingMedium
                spacing: RadarTheme.spacingSmall
                
                Text {
                    text: "🔍"
                    font.pixelSize: 14
                    color: RadarColors.textTertiary
                }
                
                TextField {
                    id: searchField
                    Layout.fillWidth: true
                    placeholderText: "Search subsystems..."
                    font.family: RadarTheme.fontFamily
                    font.pixelSize: RadarTheme.fontSizeMedium
                    color: RadarColors.textPrimary
                    
                    background: Item {}
                }
            }
        }
        
        // Subsystem list - using proper model for efficient updates
        ListView {
            id: subsystemList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: RadarTheme.spacingSmall
            
            model: subsystemManager.subsystemModel
            spacing: RadarTheme.spacingSmall
            clip: true
            
            // Disable automatic caching to prevent stale views
            cacheBuffer: 0
            
            delegate: SubsystemPaletteItem {
                width: subsystemList.width
                // Use model roles directly instead of modelData
                subsystemId: model.id
                subsystemName: model.name
                subsystemType: model.type
                subsystemHealthState: model.healthState
                subsystemHealthScore: model.healthScore
                subsystemOnCanvas: model.onCanvas
                
                onAddToCanvas: {
                    subsystemManager.addToCanvas(model.id)
                }
            }
            
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }
        }
        
        // Footer - Summary
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: RadarColors.backgroundMedium
            
            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: RadarColors.border
            }
            
            Column {
                anchors.centerIn: parent
                spacing: RadarTheme.spacingSmall
                
                Row {
                    spacing: RadarTheme.spacingLarge
                    
                    StatusIndicator {
                        label: "OK"
                        count: subsystemManager.healthySubsystemCount
                        color: RadarColors.healthOk
                    }
                    
                    StatusIndicator {
                        label: "WARN"
                        count: subsystemManager.degradedSubsystemCount
                        color: RadarColors.healthDegraded
                    }
                    
                    StatusIndicator {
                        label: "FAIL"
                        count: subsystemManager.failedSubsystemCount
                        color: RadarColors.healthFail
                    }
                }
            }
        }
    }
    
    // Helper component for status indicators
    component StatusIndicator: Row {
        property string label
        property int count
        property color color
        
        spacing: 4
        
        Rectangle {
            width: 10
            height: 10
            radius: 5
            color: parent.color
            anchors.verticalCenter: parent.verticalCenter
        }
        
        Text {
            text: count
            font.family: RadarTheme.fontFamilyMono
            font.pixelSize: RadarTheme.fontSizeMedium
            font.bold: true
            color: RadarColors.textPrimary
        }
    }
    
    /**
     * Individual subsystem item in the palette
     * Uses individual properties instead of object for efficient model binding
     */
    component SubsystemPaletteItem: Rectangle {
        id: paletteItem
        
        // Use individual properties for efficient model binding
        property string subsystemId: ""
        property string subsystemName: ""
        property string subsystemType: ""
        property string subsystemHealthState: "UNKNOWN"
        property real subsystemHealthScore: 100
        property bool subsystemOnCanvas: false
        
        signal addToCanvas()
        
        height: 72
        radius: RadarTheme.radiusMedium
        color: mouseArea.containsMouse ? RadarColors.surfaceHover : RadarColors.surface
        border.color: subsystemOnCanvas ? RadarColors.accent : RadarColors.border
        border.width: subsystemOnCanvas ? 2 : 1
        
        opacity: subsystemOnCanvas ? 0.6 : 1.0
        
        Behavior on color {
            ColorAnimation { duration: RadarTheme.animationFast }
        }
        
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            
            drag.target: !paletteItem.subsystemOnCanvas ? dragProxy : null
            drag.threshold: 10
            
            onClicked: {
                if (!paletteItem.subsystemOnCanvas && !drag.active) {
                    paletteItem.addToCanvas()
                }
            }
        }
        
        // Drag proxy - using Automatic drag type for proper handling
        Rectangle {
            id: dragProxy
            width: 48
            height: 48
            radius: RadarTheme.radiusMedium
            color: RadarColors.getHealthGlowColor(paletteItem.subsystemHealthState)
            border.color: RadarColors.getHealthColor(paletteItem.subsystemHealthState)
            border.width: 2
            visible: Drag.active
            opacity: 0.8
            
            Drag.active: mouseArea.drag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction
            Drag.keys: ["subsystemId"]
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2
            
            property string subsystemIdValue: paletteItem.subsystemId
            Drag.mimeData: ({ "text/plain": subsystemIdValue, "subsystemId": subsystemIdValue })
            
            Text {
                anchors.centerIn: parent
                text: getSubsystemIcon(paletteItem.subsystemType)
                font.pixelSize: 20
            }
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: RadarTheme.spacingMedium
            spacing: RadarTheme.spacingMedium
            
            // Health indicator and icon
            Rectangle {
                width: 48
                height: 48
                radius: RadarTheme.radiusMedium
                color: RadarColors.getHealthGlowColor(paletteItem.subsystemHealthState)
                border.color: RadarColors.getHealthColor(paletteItem.subsystemHealthState)
                border.width: 2
                
                Text {
                    anchors.centerIn: parent
                    text: getSubsystemIcon(paletteItem.subsystemType)
                    font.pixelSize: 20
                }
            }
            
            // Info
            Column {
                Layout.fillWidth: true
                spacing: 2
                
                Text {
                    text: paletteItem.subsystemName
                    font.family: RadarTheme.fontFamily
                    font.pixelSize: RadarTheme.fontSizeMedium
                    font.weight: Font.Medium
                    color: RadarColors.textPrimary
                    elide: Text.ElideRight
                    width: parent.width
                }
                
                Text {
                    text: paletteItem.subsystemType
                    font.family: RadarTheme.fontFamily
                    font.pixelSize: RadarTheme.fontSizeSmall
                    color: RadarColors.textTertiary
                }
                
                Row {
                    spacing: RadarTheme.spacingSmall
                    
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: RadarColors.getHealthColor(paletteItem.subsystemHealthState)
                    }
                    
                    Text {
                        text: paletteItem.subsystemHealthState + " - " + paletteItem.subsystemHealthScore.toFixed(0) + "%"
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeXSmall
                        color: RadarColors.getHealthColor(paletteItem.subsystemHealthState)
                    }
                }
            }
            
            // Status / Add button
            Rectangle {
                width: 28
                height: 28
                radius: 14
                color: paletteItem.subsystemOnCanvas ? RadarColors.accent : "transparent"
                border.color: paletteItem.subsystemOnCanvas ? RadarColors.accent : RadarColors.border
                border.width: 1
                visible: !paletteItem.subsystemOnCanvas || mouseArea.containsMouse
                
                Text {
                    anchors.centerIn: parent
                    text: paletteItem.subsystemOnCanvas ? "✓" : "+"
                    font.pixelSize: 14
                    font.bold: true
                    color: paletteItem.subsystemOnCanvas ? RadarColors.background : RadarColors.textSecondary
                }
            }
        }
        
        function getSubsystemIcon(type) {
            switch(type) {
                case "Transmitter": return "📡"
                case "Receiver": return "📻"
                case "Antenna & Servo": return "🎯"
                case "RF Front-End": return "📶"
                case "Signal Processor": return "🔬"
                case "Data Processor": return "💻"
                case "Power Supply": return "⚡"
                case "Cooling System": return "❄️"
                case "Timing & Sync": return "⏱️"
                case "Network Interface": return "🌐"
                default: return "❓"
            }
        }
    }
}
