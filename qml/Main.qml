import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import "styles"
import "components"
import "panels"

/**
 * Main application window for the Radar Maintenance Processor
 * Defence-grade radar health monitoring HMI
 */
ApplicationWindow {
    id: mainWindow
    
    visible: true
    width: 1920
    height: 1080
    minimumWidth: 1280
    minimumHeight: 800
    
    title: qsTr("Radar Maintenance Processor - Health Monitoring System")
    
    color: RadarColors.background
    
    // System-wide properties
    property var selectedSubsystem: null
    property bool showDetailPanel: false
    property string currentPanel: "overview"
    
    // Header bar
    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: RadarTheme.headerHeight
        color: RadarColors.backgroundDark
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: RadarTheme.spacingLarge
            anchors.rightMargin: RadarTheme.spacingLarge
            spacing: RadarTheme.spacingLarge
            
            // Logo and title
            Row {
                spacing: RadarTheme.spacingMedium
                
                Rectangle {
                    width: 40
                    height: 40
                    radius: 8
                    color: RadarColors.accent
                    
                    Text {
                        anchors.centerIn: parent
                        text: "RMP"
                        font.pixelSize: 14
                        font.bold: true
                        color: RadarColors.background
                    }
                }
                
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    
                    Text {
                        text: "RADAR MAINTENANCE PROCESSOR"
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeLarge
                        font.weight: Font.Bold
                        font.letterSpacing: 2
                        color: RadarColors.textPrimary
                    }
                    
                    Text {
                        text: "Health Monitoring System v1.0"
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeSmall
                        color: RadarColors.textSecondary
                    }
                }
            }
            
            Item { Layout.fillWidth: true }
            
            // System health summary
            Row {
                spacing: RadarTheme.spacingXLarge
                
                // System status
                Row {
                    spacing: RadarTheme.spacingSmall
                    
                    Rectangle {
                        width: RadarTheme.healthIndicatorLarge
                        height: RadarTheme.healthIndicatorLarge
                        radius: width / 2
                        color: RadarColors.getHealthColor(subsystemManager.systemHealthState)
                        
                        SequentialAnimation on opacity {
                            running: subsystemManager.systemHealthState === "FAIL"
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.4; duration: 500 }
                            NumberAnimation { to: 1.0; duration: 500 }
                        }
                    }
                    
                    Column {
                        Text {
                            text: "SYSTEM STATUS"
                            font.family: RadarTheme.fontFamily
                            font.pixelSize: RadarTheme.fontSizeXSmall
                            color: RadarColors.textTertiary
                        }
                        Text {
                            text: subsystemManager.systemHealthState
                            font.family: RadarTheme.fontFamily
                            font.pixelSize: RadarTheme.fontSizeMedium
                            font.bold: true
                            color: RadarColors.getHealthColor(subsystemManager.systemHealthState)
                        }
                    }
                }
                
                // Health score
                Column {
                    Text {
                        text: "HEALTH SCORE"
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeXSmall
                        color: RadarColors.textTertiary
                    }
                    Text {
                        text: subsystemManager.systemHealthScore.toFixed(1) + "%"
                        font.family: RadarTheme.fontFamilyMono
                        font.pixelSize: RadarTheme.fontSizeLarge
                        font.bold: true
                        color: RadarColors.textPrimary
                    }
                }
                
                // Active faults
                Column {
                    Text {
                        text: "ACTIVE FAULTS"
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeXSmall
                        color: RadarColors.textTertiary
                    }
                    Text {
                        text: subsystemManager.faultManager.totalActiveFaults
                        font.family: RadarTheme.fontFamilyMono
                        font.pixelSize: RadarTheme.fontSizeLarge
                        font.bold: true
                        color: subsystemManager.faultManager.totalActiveFaults > 0 
                               ? RadarColors.healthFail : RadarColors.textPrimary
                    }
                }
                
                // Time
                Column {
                    Text {
                        text: "SYSTEM TIME"
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeXSmall
                        color: RadarColors.textTertiary
                    }
                    Text {
                        id: timeDisplay
                        font.family: RadarTheme.fontFamilyMono
                        font.pixelSize: RadarTheme.fontSizeMedium
                        color: RadarColors.textPrimary
                        
                        Timer {
                            interval: 1000
                            running: true
                            repeat: true
                            onTriggered: {
                                timeDisplay.text = new Date().toLocaleTimeString(Qt.locale(), "HH:mm:ss")
                            }
                        }
                        
                        Component.onCompleted: {
                            text = new Date().toLocaleTimeString(Qt.locale(), "HH:mm:ss")
                        }
                    }
                }
            }
        }
        
        // Bottom border
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: RadarColors.border
        }
    }
    
    // Main content area
    RowLayout {
        anchors.top: header.bottom
        anchors.bottom: statusBar.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0
        
        // Left panel - Subsystem palette
        SubsystemPalette {
            id: palette
            Layout.preferredWidth: RadarTheme.paletteWidth
            Layout.fillHeight: true
        }
        
        // Separator
        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: RadarColors.border
        }
        
        // Central canvas area
        SystemCanvas {
            id: canvas
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            onSubsystemSelected: function(subsystemId) {
                mainWindow.selectedSubsystem = subsystemManager.getSubsystemById(subsystemId)
                mainWindow.showDetailPanel = true
            }
        }
        
        // Separator
        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: RadarColors.border
            visible: showDetailPanel
        }
        
        // Right panel - Details / Analytics
        Loader {
            id: rightPanelLoader
            Layout.preferredWidth: showDetailPanel ? RadarTheme.panelWidth : 0
            Layout.fillHeight: true
            visible: showDetailPanel
            
            sourceComponent: selectedSubsystem ? detailPanelComponent : null
            
            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: RadarTheme.animationMedium }
            }
        }
    }
    
    // Status bar
    Rectangle {
        id: statusBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: RadarTheme.statusBarHeight
        color: RadarColors.backgroundDark
        
        // Top border
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: RadarColors.border
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: RadarTheme.spacingMedium
            anchors.rightMargin: RadarTheme.spacingMedium
            spacing: RadarTheme.spacingXLarge
            
            // Simulator status
            Row {
                spacing: RadarTheme.spacingSmall
                
                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: healthSimulator.running ? RadarColors.healthOk : RadarColors.textTertiary
                }
                
                Text {
                    text: "Simulator: " + (healthSimulator.running ? "Running" : "Stopped")
                    font.family: RadarTheme.fontFamily
                    font.pixelSize: RadarTheme.fontSizeSmall
                    color: RadarColors.textSecondary
                }
            }
            
            // Scenario
            Text {
                text: "Scenario: " + healthSimulator.scenario
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeSmall
                color: RadarColors.textSecondary
            }
            
            // Subsystem count
            Text {
                text: "Active: " + subsystemManager.activeSubsystemCount + "/" + subsystemManager.totalSubsystemCount
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeSmall
                color: RadarColors.textSecondary
            }
            
            Item { Layout.fillWidth: true }
            
            // Simulator controls
            Row {
                spacing: RadarTheme.spacingSmall
                
                Button {
                    text: healthSimulator.running ? "⏸" : "▶"
                    width: 32
                    height: 24
                    onClicked: {
                        if (healthSimulator.running) {
                            healthSimulator.stop()
                        } else {
                            healthSimulator.start()
                        }
                    }
                    
                    background: Rectangle {
                        color: parent.hovered ? RadarColors.surfaceHover : RadarColors.surface
                        radius: RadarTheme.radiusSmall
                        border.color: RadarColors.border
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 12
                        color: RadarColors.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                ComboBox {
                    id: scenarioCombo
                    width: 120
                    height: 24
                    model: ["Normal", "Degraded", "HighStress", "ThermalStress", "PowerIssues", "PartialFailure"]
                    currentIndex: 0
                    
                    onActivated: {
                        healthSimulator.scenario = currentText
                    }
                    
                    background: Rectangle {
                        color: RadarColors.surface
                        radius: RadarTheme.radiusSmall
                        border.color: RadarColors.border
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: scenarioCombo.displayText
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeSmall
                        color: RadarColors.textPrimary
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 8
                    }
                }
            }
            
            // Version
            Text {
                text: "v1.0.0"
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeSmall
                color: RadarColors.textTertiary
            }
        }
    }
    
    // Detail panel component
    Component {
        id: detailPanelComponent
        
        DetailedHealthPanel {
            subsystem: mainWindow.selectedSubsystem
            
            onCloseRequested: {
                mainWindow.showDetailPanel = false
                mainWindow.selectedSubsystem = null
            }
        }
    }
    
    // Keyboard shortcuts
    Shortcut {
        sequence: "Escape"
        onActivated: {
            showDetailPanel = false
            selectedSubsystem = null
        }
    }
    
    Shortcut {
        sequence: "Space"
        onActivated: {
            if (healthSimulator.running) {
                healthSimulator.stop()
            } else {
                healthSimulator.start()
            }
        }
    }
}
