import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../styles"

/**
 * Central system canvas showing active radar subsystem modules
 * Supports drag-and-drop module placement
 */
Rectangle {
    id: canvas
    
    signal subsystemSelected(string subsystemId)
    
    color: RadarColors.background
    
    // Grid background pattern
    Canvas {
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            
            ctx.strokeStyle = RadarColors.surfaceBorder
            ctx.lineWidth = 0.5
            
            var gridSize = 40
            
            // Draw vertical lines
            for (var x = 0; x < width; x += gridSize) {
                ctx.beginPath()
                ctx.moveTo(x, 0)
                ctx.lineTo(x, height)
                ctx.stroke()
            }
            
            // Draw horizontal lines
            for (var y = 0; y < height; y += gridSize) {
                ctx.beginPath()
                ctx.moveTo(0, y)
                ctx.lineTo(width, y)
                ctx.stroke()
            }
        }
    }
    
    // Header
    Rectangle {
        id: canvasHeader
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 48
        color: RadarColors.backgroundMedium
        z: 10
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: RadarTheme.spacingLarge
            anchors.rightMargin: RadarTheme.spacingLarge
            
            Text {
                text: "SYSTEM CONFIGURATION"
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeSmall
                font.weight: Font.Bold
                font.letterSpacing: 1
                color: RadarColors.textSecondary
            }
            
            Item { Layout.fillWidth: true }
            
            Text {
                text: subsystemManager.activeSubsystemCount + " modules active"
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeSmall
                color: RadarColors.textTertiary
            }
            
            // View controls
            Row {
                spacing: RadarTheme.spacingSmall
                
                Button {
                    text: "Reset Layout"
                    height: 28
                    onClicked: resetLayout()
                    
                    background: Rectangle {
                        color: parent.hovered ? RadarColors.surfaceHover : RadarColors.surface
                        radius: RadarTheme.radiusSmall
                        border.color: RadarColors.border
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeSmall
                        color: RadarColors.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
        
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: RadarColors.border
        }
    }
    
    // Module container with FlowLayout
    Flickable {
        id: moduleContainer
        anchors.top: canvasHeader.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: RadarTheme.spacingLarge
        
        contentWidth: moduleFlow.width
        contentHeight: moduleFlow.height
        clip: true
        
        Flow {
            id: moduleFlow
            width: moduleContainer.width
            spacing: RadarTheme.spacingLarge
            
            Repeater {
                id: moduleRepeater
                model: subsystemManager.activeSubsystems
                
                SubsystemModule {
                    subsystemData: modelData
                    
                    onClicked: {
                        canvas.subsystemSelected(modelData.id)
                    }
                    
                    onRemoveRequested: {
                        subsystemManager.removeFromCanvas(modelData.id)
                    }
                }
            }
        }
        
        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }
    }
    
    // Empty state
    Column {
        anchors.centerIn: parent
        spacing: RadarTheme.spacingLarge
        visible: subsystemManager.activeSubsystemCount === 0
        
        Text {
            text: "📡"
            font.pixelSize: 64
            anchors.horizontalCenter: parent.horizontalCenter
            opacity: 0.3
        }
        
        Text {
            text: "No Subsystems Active"
            font.family: RadarTheme.fontFamily
            font.pixelSize: RadarTheme.fontSizeXLarge
            color: RadarColors.textTertiary
            anchors.horizontalCenter: parent.horizontalCenter
        }
        
        Text {
            text: "Drag modules from the palette or click + to add"
            font.family: RadarTheme.fontFamily
            font.pixelSize: RadarTheme.fontSizeMedium
            color: RadarColors.textTertiary
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
    
    // Drop area for drag-and-drop
    DropArea {
        anchors.fill: parent
        
        onDropped: function(drop) {
            if (drop.hasText || drop.keys.indexOf("subsystemId") >= 0) {
                var subsystemId = drop.text || drop.getDataAsString("subsystemId")
                subsystemManager.addToCanvas(subsystemId)
            }
        }
        
        onEntered: function(drag) {
            drag.accepted = true
        }
    }
    
    function resetLayout() {
        // Reset module positions - the Flow layout handles this automatically
        moduleFlow.spacing = RadarTheme.spacingLarge
    }
}
