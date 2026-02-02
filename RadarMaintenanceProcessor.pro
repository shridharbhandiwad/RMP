#-------------------------------------------------
#
# Radar Maintenance Processor (RMP)
# Modular Radar Health Monitoring System
#
# Qt 6 Project File
#
#-------------------------------------------------

QT += core gui qml quick quickcontrols2 charts network

greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

CONFIG += c++17
CONFIG += qmltypes

# Application info
TARGET = RadarMaintenanceProcessor
TEMPLATE = app
VERSION = 1.0.0

# QML module registration
QML_IMPORT_NAME = RadarRMP
QML_IMPORT_MAJOR_VERSION = 1

# Compiler flags
QMAKE_CXXFLAGS += -std=c++17

# Define application version
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Include paths
INCLUDEPATH += \
    $$PWD/include \
    $$PWD/include/core \
    $$PWD/include/subsystems \
    $$PWD/include/simulator \
    $$PWD/include/analytics

#-------------------------------------------------
# Header Files
#-------------------------------------------------

HEADERS += \
    # Core
    include/core/HealthStatus.h \
    include/core/IRadarSubsystem.h \
    include/core/RadarSubsystem.h \
    include/core/SubsystemManager.h \
    include/core/HealthDataPipeline.h \
    include/core/TelemetryData.h \
    include/core/FaultManager.h \
    # Subsystems
    include/subsystems/TransmitterSubsystem.h \
    include/subsystems/ReceiverSubsystem.h \
    include/subsystems/AntennaServoSubsystem.h \
    include/subsystems/RFFrontEndSubsystem.h \
    include/subsystems/SignalProcessorSubsystem.h \
    include/subsystems/DataProcessorSubsystem.h \
    include/subsystems/PowerSupplySubsystem.h \
    include/subsystems/CoolingSubsystem.h \
    include/subsystems/TimingSyncSubsystem.h \
    include/subsystems/NetworkInterfaceSubsystem.h \
    # Simulator
    include/simulator/HealthSimulator.h \
    include/simulator/FaultInjector.h \
    include/simulator/TelemetryGenerator.h \
    # Analytics
    include/analytics/HealthAnalytics.h \
    include/analytics/TrendAnalyzer.h \
    include/analytics/UptimeTracker.h

#-------------------------------------------------
# Source Files
#-------------------------------------------------

SOURCES += \
    # Main
    src/main.cpp \
    # Core
    src/core/RadarSubsystem.cpp \
    src/core/SubsystemManager.cpp \
    src/core/HealthDataPipeline.cpp \
    src/core/TelemetryData.cpp \
    src/core/FaultManager.cpp \
    # Subsystems
    src/subsystems/TransmitterSubsystem.cpp \
    src/subsystems/ReceiverSubsystem.cpp \
    src/subsystems/AntennaServoSubsystem.cpp \
    src/subsystems/RFFrontEndSubsystem.cpp \
    src/subsystems/SignalProcessorSubsystem.cpp \
    src/subsystems/DataProcessorSubsystem.cpp \
    src/subsystems/PowerSupplySubsystem.cpp \
    src/subsystems/CoolingSubsystem.cpp \
    src/subsystems/TimingSyncSubsystem.cpp \
    src/subsystems/NetworkInterfaceSubsystem.cpp \
    # Simulator
    src/simulator/HealthSimulator.cpp \
    src/simulator/FaultInjector.cpp \
    src/simulator/TelemetryGenerator.cpp \
    # Analytics
    src/analytics/HealthAnalytics.cpp \
    src/analytics/TrendAnalyzer.cpp \
    src/analytics/UptimeTracker.cpp

#-------------------------------------------------
# Resources
#-------------------------------------------------

RESOURCES += \
    qml.qrc

#-------------------------------------------------
# QML Files (for IDE support)
#-------------------------------------------------

DISTFILES += \
    # Main
    qml/Main.qml \
    # Components
    qml/components/SystemCanvas.qml \
    qml/components/SubsystemPalette.qml \
    qml/components/SubsystemModule.qml \
    qml/components/HealthIndicator.qml \
    qml/components/TelemetryDisplay.qml \
    qml/components/FaultList.qml \
    # Modules
    qml/modules/TransmitterModule.qml \
    qml/modules/ReceiverModule.qml \
    qml/modules/AntennaServoModule.qml \
    qml/modules/RFFrontEndModule.qml \
    qml/modules/SignalProcessorModule.qml \
    qml/modules/DataProcessorModule.qml \
    qml/modules/PowerSupplyModule.qml \
    qml/modules/CoolingModule.qml \
    qml/modules/TimingSyncModule.qml \
    qml/modules/NetworkInterfaceModule.qml \
    # Panels
    qml/panels/DetailedHealthPanel.qml \
    qml/panels/AnalyticsPanel.qml \
    qml/panels/SystemOverviewPanel.qml \
    qml/panels/FaultHistoryPanel.qml \
    # Styles
    qml/styles/RadarColors.qml \
    qml/styles/RadarTheme.qml

#-------------------------------------------------
# Additional Configuration
#-------------------------------------------------

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Copy QML files to build directory
CONFIG(debug, debug|release) {
    DESTDIR = $$OUT_PWD/debug
} else {
    DESTDIR = $$OUT_PWD/release
}

# Platform-specific settings
win32 {
    # Windows-specific settings
    RC_ICONS = resources/icons/app.ico
}

macx {
    # macOS-specific settings
    ICON = resources/icons/app.icns
    QMAKE_INFO_PLIST = resources/Info.plist
}

unix:!macx {
    # Linux-specific settings
    QMAKE_LFLAGS += -Wl,-rpath,\'\$$ORIGIN\'
}

#-------------------------------------------------
# Build Output
#-------------------------------------------------

# Object files directory
OBJECTS_DIR = $$OUT_PWD/.obj

# MOC files directory
MOC_DIR = $$OUT_PWD/.moc

# RCC files directory  
RCC_DIR = $$OUT_PWD/.rcc

# UI files directory
UI_DIR = $$OUT_PWD/.ui
