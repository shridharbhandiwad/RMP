import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../styles"
import "../components"

/**
 * Detailed health panel showing comprehensive subsystem information
 */
Rectangle {
    id: panel
    
    property var subsystem
    
    signal closeRequested()
    
    color: RadarColors.backgroundLight
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            color: RadarColors.backgroundMedium
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: RadarTheme.spacingMedium
                anchors.rightMargin: RadarTheme.spacingMedium
                spacing: RadarTheme.spacingMedium
                
                // Health indicator
                HealthIndicator {
                    healthState: subsystem ? subsystem.healthState : "UNKNOWN"
                    width: 24
                    height: 24
                }
                
                // Title
                Column {
                    Layout.fillWidth: true
                    
                    Text {
                        text: subsystem ? subsystem.name : "Unknown"
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeLarge
                        font.weight: Font.Medium
                        color: RadarColors.textPrimary
                    }
                    
                    Text {
                        text: subsystem ? subsystem.type : ""
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeSmall
                        color: RadarColors.textTertiary
                    }
                }
                
                // Close button
                Rectangle {
                    width: 32
                    height: 32
                    radius: 16
                    color: closeMouseArea.containsMouse ? RadarColors.surfaceHover : "transparent"
                    
                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        font.pixelSize: 20
                        color: RadarColors.textSecondary
                    }
                    
                    MouseArea {
                        id: closeMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: panel.closeRequested()
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
        
        // Content tabs
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            
            background: Rectangle {
                color: RadarColors.backgroundMedium
            }
            
            Repeater {
                model: ["Status", "Telemetry", "Faults", "Trends"]
                
                TabButton {
                    text: modelData
                    width: implicitWidth
                    
                    background: Rectangle {
                        color: tabBar.currentIndex === index ? RadarColors.surface : "transparent"
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeSmall
                        color: tabBar.currentIndex === index ? RadarColors.accent : RadarColors.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
        
        // Tab content
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex
            
            // Status tab
            StatusTab {
                subsystem: panel.subsystem
            }
            
            // Telemetry tab
            TelemetryTab {
                subsystem: panel.subsystem
            }
            
            // Faults tab
            FaultsTab {
                subsystem: panel.subsystem
            }
            
            // Trends tab
            TrendsTab {
                subsystem: panel.subsystem
            }
        }
    }
    
    // Status tab component
    component StatusTab: Flickable {
        id: statusFlickable
        property var subsystem
        
        contentHeight: statusContent.height
        clip: true
        
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
        
        Column {
            id: statusContent
            width: statusFlickable.width
            padding: RadarTheme.spacingMedium
            spacing: RadarTheme.spacingLarge
            
            // Health score card
            Rectangle {
                width: parent.width - parent.padding * 2
                height: 100
                radius: RadarTheme.radiusMedium
                color: RadarColors.surface
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: RadarTheme.spacingLarge
                    
                    Column {
                        Layout.fillWidth: true
                        spacing: RadarTheme.spacingSmall
                        
                        Text {
                            text: "HEALTH SCORE"
                            font.family: RadarTheme.fontFamily
                            font.pixelSize: RadarTheme.fontSizeXSmall
                            font.letterSpacing: 1
                            color: RadarColors.textTertiary
                        }
                        
                        Text {
                            text: subsystem ? subsystem.healthScore.toFixed(1) + "%" : "0%"
                            font.family: RadarTheme.fontFamilyMono
                            font.pixelSize: RadarTheme.fontSizeHuge
                            font.bold: true
                            color: subsystem ? RadarColors.getHealthColor(subsystem.healthState) : RadarColors.textTertiary
                        }
                    }
                    
                    // Circular progress
                    Item {
                        width: 64
                        height: 64
                        
                        Canvas {
                            id: progressCanvas
                            anchors.fill: parent
                            
                            property real progress: subsystem ? subsystem.healthScore / 100 : 0
                            
                            onProgressChanged: requestPaint()
                            
                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.clearRect(0, 0, width, height)
                                
                                var centerX = width / 2
                                var centerY = height / 2
                                var radius = Math.min(width, height) / 2 - 4
                                
                                // Background circle
                                ctx.beginPath()
                                ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI)
                                ctx.strokeStyle = RadarColors.backgroundDark
                                ctx.lineWidth = 8
                                ctx.stroke()
                                
                                // Progress arc
                                ctx.beginPath()
                                ctx.arc(centerX, centerY, radius, -Math.PI / 2, -Math.PI / 2 + progress * 2 * Math.PI)
                                ctx.strokeStyle = subsystem ? RadarColors.getHealthColor(subsystem.healthState) : RadarColors.textTertiary
                                ctx.lineWidth = 8
                                ctx.lineCap = "round"
                                ctx.stroke()
                            }
                        }
                    }
                }
            }
            
            // Status message
            Rectangle {
                width: parent.width - parent.padding * 2
                height: 60
                radius: RadarTheme.radiusMedium
                color: RadarColors.surface
                
                Row {
                    anchors.fill: parent
                    anchors.margins: RadarTheme.spacingMedium
                    spacing: RadarTheme.spacingMedium
                    
                    Text {
                        text: "📋"
                        font.pixelSize: 24
                    }
                    
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        
                        Text {
                            text: "Status"
                            font.family: RadarTheme.fontFamily
                            font.pixelSize: RadarTheme.fontSizeXSmall
                            color: RadarColors.textTertiary
                        }
                        
                        Text {
                            text: subsystem ? subsystem.statusMessage : "Unknown"
                            font.family: RadarTheme.fontFamily
                            font.pixelSize: RadarTheme.fontSizeMedium
                            color: RadarColors.textPrimary
                        }
                    }
                }
            }
            
            // Quick stats
            Row {
                width: parent.width - parent.padding * 2
                spacing: RadarTheme.spacingMedium
                
                StatCard {
                    width: (parent.width - RadarTheme.spacingMedium) / 2
                    label: "State"
                    value: subsystem ? subsystem.healthState : "-"
                    color: subsystem ? RadarColors.getHealthColor(subsystem.healthState) : RadarColors.textTertiary
                }
                
                StatCard {
                    width: (parent.width - RadarTheme.spacingMedium) / 2
                    label: "Faults"
                    value: subsystem ? subsystem.faultCount.toString() : "0"
                    color: (subsystem && subsystem.faultCount > 0) ? RadarColors.healthFail : RadarColors.textPrimary
                }
            }
            
            // ID and type
            Rectangle {
                width: parent.width - parent.padding * 2
                height: 80
                radius: RadarTheme.radiusMedium
                color: RadarColors.surface
                
                Column {
                    anchors.fill: parent
                    anchors.margins: RadarTheme.spacingMedium
                    spacing: RadarTheme.spacingSmall
                    
                    Row {
                        spacing: RadarTheme.spacingLarge
                        
                        Column {
                            Text {
                                text: "ID"
                                font.family: RadarTheme.fontFamily
                                font.pixelSize: RadarTheme.fontSizeXSmall
                                color: RadarColors.textTertiary
                            }
                            Text {
                                text: subsystem ? subsystem.id : "-"
                                font.family: RadarTheme.fontFamilyMono
                                font.pixelSize: RadarTheme.fontSizeMedium
                                color: RadarColors.textPrimary
                            }
                        }
                        
                        Column {
                            Text {
                                text: "Type"
                                font.family: RadarTheme.fontFamily
                                font.pixelSize: RadarTheme.fontSizeXSmall
                                color: RadarColors.textTertiary
                            }
                            Text {
                                text: subsystem ? subsystem.type : "-"
                                font.family: RadarTheme.fontFamily
                                font.pixelSize: RadarTheme.fontSizeMedium
                                color: RadarColors.textPrimary
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Telemetry tab component
    component TelemetryTab: Rectangle {
        property var subsystem
        color: "transparent"
        
        TelemetryDisplay {
            anchors.fill: parent
            anchors.margins: RadarTheme.spacingMedium
            telemetryData: subsystem ? subsystem.telemetry : ({})
            maxItems: 20
        }
    }
    
    // Faults tab component
    component FaultsTab: Rectangle {
        property var subsystem
        color: "transparent"
        
        FaultList {
            anchors.fill: parent
            anchors.margins: RadarTheme.spacingMedium
            faults: subsystem ? subsystem.faults : []
        }
    }
    
    // Trends tab component (placeholder)
    component TrendsTab: Rectangle {
        property var subsystem
        color: "transparent"
        
        Column {
            anchors.centerIn: parent
            spacing: RadarTheme.spacingMedium
            
            Text {
                text: "📈"
                font.pixelSize: 48
                anchors.horizontalCenter: parent.horizontalCenter
                opacity: 0.5
            }
            
            Text {
                text: "Trend Analysis"
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeLarge
                color: RadarColors.textTertiary
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            Text {
                text: "Historical data visualization\ncoming in future release"
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeSmall
                color: RadarColors.textTertiary
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
    
    // Stat card component
    component StatCard: Rectangle {
        property string label
        property string value
        property color color: RadarColors.textPrimary
        
        height: 70
        radius: RadarTheme.radiusMedium
        color: RadarColors.surface
        
        Column {
            anchors.centerIn: parent
            spacing: 4
            
            Text {
                text: label
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeXSmall
                color: RadarColors.textTertiary
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            Text {
                text: value
                font.family: RadarTheme.fontFamilyMono
                font.pixelSize: RadarTheme.fontSizeXLarge
                font.bold: true
                color: parent.parent.color
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
