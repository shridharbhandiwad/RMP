import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../styles"

/**
 * Analytics panel showing system-wide health metrics
 */
Rectangle {
    id: panel
    
    color: RadarColors.backgroundLight
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: RadarColors.backgroundMedium
            
            Text {
                anchors.centerIn: parent
                text: "SYSTEM ANALYTICS"
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeMedium
                font.weight: Font.Bold
                font.letterSpacing: 1
                color: RadarColors.textSecondary
            }
            
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: RadarColors.border
            }
        }
        
        // Content
        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentHeight: contentColumn.height
            clip: true
            
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            
            Column {
                id: contentColumn
                width: parent.width
                padding: RadarTheme.spacingMedium
                spacing: RadarTheme.spacingLarge
                
                // System availability
                AnalyticsCard {
                    width: parent.width - parent.padding * 2
                    title: "System Availability"
                    value: healthAnalytics.systemAvailability.toFixed(2) + "%"
                    subtitle: "Last 24 hours"
                }
                
                // Average health
                AnalyticsCard {
                    width: parent.width - parent.padding * 2
                    title: "Average Health Score"
                    value: healthAnalytics.averageHealthScore.toFixed(1) + "%"
                    subtitle: "Across all subsystems"
                }
                
                // Total faults
                AnalyticsCard {
                    width: parent.width - parent.padding * 2
                    title: "Total Faults"
                    value: healthAnalytics.totalFaults.toString()
                    subtitle: "Since tracking started"
                }
            }
        }
    }
    
    component AnalyticsCard: Rectangle {
        property string title
        property string value
        property string subtitle
        
        height: 100
        radius: RadarTheme.radiusMedium
        color: RadarColors.surface
        
        Column {
            anchors.fill: parent
            anchors.margins: RadarTheme.spacingMedium
            spacing: RadarTheme.spacingSmall
            
            Text {
                text: title
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeSmall
                color: RadarColors.textTertiary
            }
            
            Text {
                text: value
                font.family: RadarTheme.fontFamilyMono
                font.pixelSize: RadarTheme.fontSizeXXLarge
                font.bold: true
                color: RadarColors.textPrimary
            }
            
            Text {
                text: subtitle
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeXSmall
                color: RadarColors.textTertiary
            }
        }
    }
}
