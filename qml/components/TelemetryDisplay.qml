import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../styles"

/**
 * Displays telemetry parameters with value bars
 */
Item {
    id: display
    
    property var telemetryData: ({})
    property var metadata: ({})
    property int maxItems: 8
    
    implicitHeight: telemetryColumn.implicitHeight
    
    ColumnLayout {
        id: telemetryColumn
        anchors.fill: parent
        spacing: RadarTheme.spacingSmall
        
        Repeater {
            model: {
                var items = []
                var keys = Object.keys(telemetryData)
                for (var i = 0; i < Math.min(keys.length, maxItems); i++) {
                    items.push({
                        key: keys[i],
                        value: telemetryData[keys[i]]
                    })
                }
                return items
            }
            
            TelemetryItem {
                Layout.fillWidth: true
                paramName: modelData.key
                paramValue: modelData.value
                paramMetadata: metadata[modelData.key] || {}
            }
        }
    }
    
    component TelemetryItem: Rectangle {
        id: item
        
        property string paramName
        property var paramValue
        property var paramMetadata
        
        height: 44
        radius: RadarTheme.radiusSmall
        color: RadarColors.backgroundDark
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: RadarTheme.spacingMedium
            anchors.rightMargin: RadarTheme.spacingMedium
            spacing: RadarTheme.spacingMedium
            
            // Parameter name
            Text {
                Layout.preferredWidth: 100
                text: formatParamName(paramName)
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeSmall
                color: RadarColors.textSecondary
                elide: Text.ElideRight
            }
            
            // Value bar
            Rectangle {
                Layout.fillWidth: true
                height: 6
                radius: 3
                color: RadarColors.background
                visible: isNumeric(paramValue)
                
                Rectangle {
                    width: parent.width * getValueRatio()
                    height: parent.height
                    radius: 3
                    color: getValueColor()
                    
                    Behavior on width {
                        NumberAnimation { duration: RadarTheme.animationMedium }
                    }
                }
            }
            
            // Value display
            Text {
                Layout.preferredWidth: 80
                text: formatValue(paramValue)
                font.family: RadarTheme.fontFamilyMono
                font.pixelSize: RadarTheme.fontSizeSmall
                color: getValueColor()
                horizontalAlignment: Text.AlignRight
            }
            
            // Unit
            Text {
                Layout.preferredWidth: 40
                text: paramMetadata.unit || ""
                font.family: RadarTheme.fontFamily
                font.pixelSize: RadarTheme.fontSizeXSmall
                color: RadarColors.textTertiary
            }
        }
        
        function formatParamName(name) {
            // Convert camelCase to Title Case
            return name.replace(/([A-Z])/g, ' $1').replace(/^./, function(str) {
                return str.toUpperCase()
            })
        }
        
        function formatValue(value) {
            if (typeof value === 'number') {
                return value.toFixed(2)
            } else if (typeof value === 'boolean') {
                return value ? "ON" : "OFF"
            }
            return String(value)
        }
        
        function isNumeric(value) {
            return typeof value === 'number'
        }
        
        function getValueRatio() {
            if (!isNumeric(paramValue)) return 0
            
            var min = paramMetadata.minValue !== undefined ? paramMetadata.minValue : 0
            var max = paramMetadata.maxValue !== undefined ? paramMetadata.maxValue : 100
            
            return Math.max(0, Math.min(1, (paramValue - min) / (max - min)))
        }
        
        function getValueColor() {
            if (!isNumeric(paramValue)) {
                return RadarColors.textPrimary
            }
            
            // Check against thresholds
            if (paramMetadata.criticalHigh !== undefined && paramValue >= paramMetadata.criticalHigh) {
                return RadarColors.healthFail
            }
            if (paramMetadata.criticalLow !== undefined && paramValue <= paramMetadata.criticalLow) {
                return RadarColors.healthFail
            }
            if (paramMetadata.warningHigh !== undefined && paramValue >= paramMetadata.warningHigh) {
                return RadarColors.healthDegraded
            }
            if (paramMetadata.warningLow !== undefined && paramValue <= paramMetadata.warningLow) {
                return RadarColors.healthDegraded
            }
            
            return RadarColors.telemetryPrimary
        }
    }
}
