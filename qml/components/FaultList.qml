import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../styles"

/**
 * Displays list of active faults for a subsystem
 */
Rectangle {
    id: faultList
    
    property var faults: []
    property bool showClearButton: true
    
    signal faultClicked(string faultCode)
    signal clearFault(string faultCode)
    
    color: RadarColors.backgroundDark
    radius: RadarTheme.radiusMedium
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: RadarTheme.spacingMedium
        spacing: RadarTheme.spacingSmall
        
        // Header
        RowLayout {
            Layout.fillWidth: true
            
            Text {
                text: "ACTIVE FAULTS"
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeXSmall
                font.weight: Font.Bold
                font.letterSpacing: 1
                color: RadarColors.textTertiary
            }
            
            Item { Layout.fillWidth: true }
            
            Rectangle {
                width: 24
                height: 18
                radius: 9
                color: faults.length > 0 ? RadarColors.healthFail : RadarColors.textTertiary
                
                Text {
                    anchors.centerIn: parent
                    text: faults.length
                    font.family: RadarTheme.fontFamilyMono
                    font.pixelSize: RadarTheme.fontSizeXSmall
                    font.bold: true
                    color: "white"
                }
            }
        }
        
        // Fault list
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            model: faults
            spacing: RadarTheme.spacingSmall
            clip: true
            
            delegate: FaultItem {
                width: parent ? parent.width : 200
                fault: modelData
                showClear: showClearButton
                
                onClearClicked: faultList.clearFault(modelData.code)
                onItemClicked: faultList.faultClicked(modelData.code)
            }
            
            // Empty state
            Text {
                anchors.centerIn: parent
                text: "No active faults"
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeSmall
                color: RadarColors.textTertiary
                visible: faults.length === 0
            }
        }
    }
    
    component FaultItem: Rectangle {
        id: faultItem
        
        property var fault
        property bool showClear: true
        
        signal clearClicked()
        signal itemClicked()
        
        height: 56
        radius: RadarTheme.radiusSmall
        color: itemMouseArea.containsMouse ? RadarColors.surfaceHover : RadarColors.surface
        border.color: getSeverityColor(fault.severity)
        border.width: 1
        
        MouseArea {
            id: itemMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: faultItem.itemClicked()
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: RadarTheme.spacingSmall
            spacing: RadarTheme.spacingSmall
            
            // Severity indicator
            Rectangle {
                width: 4
                Layout.fillHeight: true
                radius: 2
                color: getSeverityColor(fault.severity)
            }
            
            // Fault details
            Column {
                Layout.fillWidth: true
                spacing: 2
                
                Text {
                    text: fault.code
                    font.family: RadarTheme.fontFamilyMono
                    font.pixelSize: RadarTheme.fontSizeSmall
                    font.bold: true
                    color: getSeverityColor(fault.severity)
                }
                
                Text {
                    text: fault.description
                    font.family: RadarTheme.fontFamily
                    font.pixelSize: RadarTheme.fontSizeSmall
                    color: RadarColors.textPrimary
                    elide: Text.ElideRight
                    width: parent.width
                }
                
                Text {
                    text: formatTimestamp(fault.timestamp)
                    font.family: RadarTheme.fontFamily
                    font.pixelSize: RadarTheme.fontSizeXSmall
                    color: RadarColors.textTertiary
                }
            }
            
            // Clear button
            Rectangle {
                width: 24
                height: 24
                radius: 12
                color: clearMouseArea.containsMouse ? RadarColors.surfaceHover : "transparent"
                visible: showClear && itemMouseArea.containsMouse
                
                Text {
                    anchors.centerIn: parent
                    text: "×"
                    font.pixelSize: 14
                    color: RadarColors.textSecondary
                }
                
                MouseArea {
                    id: clearMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: faultItem.clearClicked()
                }
            }
        }
        
        function getSeverityColor(severity) {
            switch(severity) {
                case "INFO": return RadarColors.severityInfo
                case "WARNING": return RadarColors.severityWarning
                case "CRITICAL": return RadarColors.severityCritical
                case "FATAL": return RadarColors.severityFatal
                default: return RadarColors.textTertiary
            }
        }
        
        function formatTimestamp(ts) {
            if (!ts) return ""
            var date = new Date(ts)
            return date.toLocaleTimeString(Qt.locale(), "HH:mm:ss")
        }
    }
}
