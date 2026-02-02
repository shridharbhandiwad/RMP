cmake_minimum_required(VERSION 3.16)

project(RadarMaintenanceProcessor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    Charts
    Network
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/subsystems
    ${CMAKE_CURRENT_SOURCE_DIR}/include/simulator
    ${CMAKE_CURRENT_SOURCE_DIR}/include/analytics
)

# Source files
set(CORE_SOURCES
    src/core/RadarSubsystem.cpp
    src/core/SubsystemManager.cpp
    src/core/SubsystemListModel.cpp
    src/core/HealthDataPipeline.cpp
    src/core/TelemetryData.cpp
    src/core/FaultManager.cpp
)

set(SUBSYSTEM_SOURCES
    src/subsystems/TransmitterSubsystem.cpp
    src/subsystems/ReceiverSubsystem.cpp
    src/subsystems/AntennaServoSubsystem.cpp
    src/subsystems/RFFrontEndSubsystem.cpp
    src/subsystems/SignalProcessorSubsystem.cpp
    src/subsystems/DataProcessorSubsystem.cpp
    src/subsystems/PowerSupplySubsystem.cpp
    src/subsystems/CoolingSubsystem.cpp
    src/subsystems/TimingSyncSubsystem.cpp
    src/subsystems/NetworkInterfaceSubsystem.cpp
)

set(SIMULATOR_SOURCES
    src/simulator/HealthSimulator.cpp
    src/simulator/FaultInjector.cpp
    src/simulator/TelemetryGenerator.cpp
)

set(ANALYTICS_SOURCES
    src/analytics/HealthAnalytics.cpp
    src/analytics/TrendAnalyzer.cpp
    src/analytics/UptimeTracker.cpp
)

# Header files
set(CORE_HEADERS
    include/core/IRadarSubsystem.h
    include/core/RadarSubsystem.h
    include/core/SubsystemManager.h
    include/core/SubsystemListModel.h
    include/core/HealthDataPipeline.h
    include/core/TelemetryData.h
    include/core/FaultManager.h
    include/core/HealthStatus.h
)

set(SUBSYSTEM_HEADERS
    include/subsystems/TransmitterSubsystem.h
    include/subsystems/ReceiverSubsystem.h
    include/subsystems/AntennaServoSubsystem.h
    include/subsystems/RFFrontEndSubsystem.h
    include/subsystems/SignalProcessorSubsystem.h
    include/subsystems/DataProcessorSubsystem.h
    include/subsystems/PowerSupplySubsystem.h
    include/subsystems/CoolingSubsystem.h
    include/subsystems/TimingSyncSubsystem.h
    include/subsystems/NetworkInterfaceSubsystem.h
)

set(SIMULATOR_HEADERS
    include/simulator/HealthSimulator.h
    include/simulator/FaultInjector.h
    include/simulator/TelemetryGenerator.h
)

set(ANALYTICS_HEADERS
    include/analytics/HealthAnalytics.h
    include/analytics/TrendAnalyzer.h
    include/analytics/UptimeTracker.h
)

# QML Resources
qt_add_resources(QML_RESOURCES qml.qrc)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${CORE_SOURCES}
    ${SUBSYSTEM_SOURCES}
    ${SIMULATOR_SOURCES}
    ${ANALYTICS_SOURCES}
    ${CORE_HEADERS}
    ${SUBSYSTEM_HEADERS}
    ${SIMULATOR_HEADERS}
    ${ANALYTICS_HEADERS}
    ${QML_RESOURCES}
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Charts
    Qt6::Network
)

# QML module registration
qt_add_qml_module(${PROJECT_NAME}
    URI RadarRMP
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/SystemCanvas.qml
        qml/components/SubsystemPalette.qml
        qml/components/SubsystemModule.qml
        qml/components/HealthIndicator.qml
        qml/components/TelemetryDisplay.qml
        qml/components/FaultList.qml
        qml/modules/TransmitterModule.qml
        qml/modules/ReceiverModule.qml
        qml/modules/AntennaServoModule.qml
        qml/modules/RFFrontEndModule.qml
        qml/modules/SignalProcessorModule.qml
        qml/modules/DataProcessorModule.qml
        qml/modules/PowerSupplyModule.qml
        qml/modules/CoolingModule.qml
        qml/modules/TimingSyncModule.qml
        qml/modules/NetworkInterfaceModule.qml
        qml/panels/DetailedHealthPanel.qml
        qml/panels/AnalyticsPanel.qml
        qml/panels/SystemOverviewPanel.qml
        qml/panels/FaultHistoryPanel.qml
        qml/styles/RadarTheme.qml
        qml/styles/RadarColors.qml
)

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Copy QML files to build directory for development
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/qml DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
