pragma Singleton
import QtQuick 2.15

/**
 * Radar UI Theme - Typography, spacing, and common styles
 */
QtObject {
    // Typography
    readonly property string fontFamily: "Segoe UI"
    readonly property string fontFamilyMono: "Consolas"
    
    readonly property int fontSizeXSmall: 10
    readonly property int fontSizeSmall: 11
    readonly property int fontSizeMedium: 13
    readonly property int fontSizeLarge: 16
    readonly property int fontSizeXLarge: 20
    readonly property int fontSizeXXLarge: 28
    readonly property int fontSizeHuge: 36
    
    readonly property int fontWeightLight: Font.Light
    readonly property int fontWeightNormal: Font.Normal
    readonly property int fontWeightMedium: Font.Medium
    readonly property int fontWeightBold: Font.Bold
    
    // Spacing
    readonly property int spacingXSmall: 4
    readonly property int spacingSmall: 8
    readonly property int spacingMedium: 12
    readonly property int spacingLarge: 16
    readonly property int spacingXLarge: 24
    readonly property int spacingXXLarge: 32
    
    // Border radius
    readonly property int radiusSmall: 4
    readonly property int radiusMedium: 8
    readonly property int radiusLarge: 12
    readonly property int radiusXLarge: 16
    
    // Shadows
    readonly property int shadowSmall: 2
    readonly property int shadowMedium: 4
    readonly property int shadowLarge: 8
    
    // Animation durations
    readonly property int animationFast: 150
    readonly property int animationMedium: 250
    readonly property int animationSlow: 400
    
    // Component sizes
    readonly property int moduleWidth: 220
    readonly property int moduleHeight: 160
    readonly property int moduleMinWidth: 180
    readonly property int moduleMinHeight: 140
    
    readonly property int paletteWidth: 280
    readonly property int panelWidth: 360
    readonly property int headerHeight: 56
    readonly property int statusBarHeight: 32
    
    readonly property int iconSizeSmall: 16
    readonly property int iconSizeMedium: 24
    readonly property int iconSizeLarge: 32
    readonly property int iconSizeXLarge: 48
    
    // Button sizes
    readonly property int buttonHeightSmall: 28
    readonly property int buttonHeightMedium: 36
    readonly property int buttonHeightLarge: 44
    
    // Health indicator sizes
    readonly property int healthIndicatorSmall: 8
    readonly property int healthIndicatorMedium: 12
    readonly property int healthIndicatorLarge: 16
}
