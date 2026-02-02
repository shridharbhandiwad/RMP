import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../styles"

/**
 * System overview panel with summary statistics
 */
Rectangle {
    id: panel
    
    color: RadarColors.backgroundLight
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: RadarTheme.spacingMedium
        spacing: RadarTheme.spacingMedium
        
        // System health card
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 120
            radius: RadarTheme.radiusMedium
            color: RadarColors.surface
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: RadarTheme.spacingLarge
                
                Column {
                    Layout.fillWidth: true
                    spacing: RadarTheme.spacingSmall
                    
                    Text {
                        text: "SYSTEM HEALTH"
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeXSmall
                        font.letterSpacing: 1
                        color: RadarColors.textTertiary
                    }
                    
                    Row {
                        spacing: RadarTheme.spacingMedium
                        
                        Rectangle {
                            width: 20
                            height: 20
                            radius: 10
                            color: RadarColors.getHealthColor(subsystemManager.systemHealthState)
                        }
                        
                        Text {
                            text: subsystemManager.systemHealthState
                            font.family: RadarTheme.fontFamily
                            font.pixelSize: RadarTheme.fontSizeXLarge
                            font.bold: true
                            color: RadarColors.getHealthColor(subsystemManager.systemHealthState)
                        }
                    }
                    
                    Text {
                        text: "Health Score: " + subsystemManager.systemHealthScore.toFixed(1) + "%"
                        font.family: RadarTheme.fontFamily
                        font.pixelSize: RadarTheme.fontSizeMedium
                        color: RadarColors.textSecondary
                    }
                }
            }
        }
        
        // Subsystem summary
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 100
            radius: RadarTheme.radiusMedium
            color: RadarColors.surface
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: RadarTheme.spacingMedium
                
                SummaryItem {
                    Layout.fillWidth: true
                    label: "Active"
                    value: subsystemManager.activeSubsystemCount
                    total: subsystemManager.totalSubsystemCount
                }
                
                Rectangle {
                    width: 1
                    Layout.fillHeight: true
                    color: RadarColors.border
                }
                
                SummaryItem {
                    Layout.fillWidth: true
                    label: "Healthy"
                    value: subsystemManager.healthySubsystemCount
                    color: RadarColors.healthOk
                }
                
                Rectangle {
                    width: 1
                    Layout.fillHeight: true
                    color: RadarColors.border
                }
                
                SummaryItem {
                    Layout.fillWidth: true
                    label: "Degraded"
                    value: subsystemManager.degradedSubsystemCount
                    color: RadarColors.healthDegraded
                }
                
                Rectangle {
                    width: 1
                    Layout.fillHeight: true
                    color: RadarColors.border
                }
                
                SummaryItem {
                    Layout.fillWidth: true
                    label: "Failed"
                    value: subsystemManager.failedSubsystemCount
                    color: RadarColors.healthFail
                }
            }
        }
        
        Item { Layout.fillHeight: true }
    }
    
    component SummaryItem: Column {
        property string label
        property int value
        property int total: -1
        property color color: RadarColors.textPrimary
        
        spacing: 4
        
        Text {
            text: label
            font.family: RadarTheme.fontFamily
            font.pixelSize: RadarTheme.fontSizeXSmall
            color: RadarColors.textTertiary
            anchors.horizontalCenter: parent.horizontalCenter
        }
        
        Text {
            text: total >= 0 ? value + "/" + total : value.toString()
            font.family: RadarTheme.fontFamilyMono
            font.pixelSize: RadarTheme.fontSizeXLarge
            font.bold: true
            color: parent.color
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
