import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../styles"
import "../components"

/**
 * Panel showing system-wide fault history
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
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: RadarTheme.spacingMedium
                anchors.rightMargin: RadarTheme.spacingMedium
                
                Text {
                    text: "FAULT HISTORY"
                    font.family: RadarTheme.fontFamily
                    font.pixelSize: RadarTheme.fontSizeMedium
                    font.weight: Font.Bold
                    font.letterSpacing: 1
                    color: RadarColors.textSecondary
                }
                
                Item { Layout.fillWidth: true }
                
                Text {
                    text: subsystemManager.faultManager.totalActiveFaults + " Active"
                    font.family: RadarTheme.fontFamily
                    font.pixelSize: RadarTheme.fontSizeSmall
                    color: subsystemManager.faultManager.totalActiveFaults > 0 
                           ? RadarColors.healthFail : RadarColors.textTertiary
                }
            }
            
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: RadarColors.border
            }
        }
        
        // Fault list
        FaultList {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: RadarTheme.spacingMedium
            faults: subsystemManager.faultManager.activeFaults
            showClearButton: true
            
            onClearFault: function(faultCode) {
                // Would need to find subsystem and clear fault
            }
        }
    }
}
