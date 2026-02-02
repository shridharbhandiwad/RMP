# Radar Maintenance Processor (RMP)

## Modular Radar Health Monitoring System

A comprehensive, defence-grade health monitoring and maintenance processor system for radar platforms, built with Qt 6 and QML.

![Qt](https://img.shields.io/badge/Qt-6.x-green.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![License](https://img.shields.io/badge/License-Proprietary-red.svg)

---

## 🎯 Overview

The Radar Maintenance Processor (RMP) is a modular, composable system for monitoring and managing radar subsystem health in real-time. It provides:

- **Real-time health monitoring** across 10 radar subsystem types
- **Drag-and-drop module configuration** for flexible system layouts
- **Comprehensive fault management** with severity classification
- **Trend analysis and predictive insights**
- **Defence-grade dark UI** suitable for command center environments

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        RADAR MAINTENANCE PROCESSOR                       │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐              │
│  │   QML/Qt     │◄──►│  Subsystem   │◄──►│   Health     │              │
│  │   Frontend   │    │   Manager    │    │   Pipeline   │              │
│  └──────────────┘    └──────────────┘    └──────────────┘              │
│         │                   │                    │                       │
│         │                   ▼                    │                       │
│         │         ┌──────────────────┐           │                       │
│         │         │  Radar Subsystems │           │                       │
│         │         ├──────────────────┤           │                       │
│         │         │ • Transmitter    │           │                       │
│         │         │ • Receiver       │           │                       │
│         │         │ • Antenna/Servo  │           │                       │
│         │         │ • RF Front-End   │           │                       │
│         │         │ • Signal Proc.   │           │                       │
│         │         │ • Data Proc.     │           │                       │
│         │         │ • Power Supply   │           │                       │
│         │         │ • Cooling        │           │                       │
│         │         │ • Timing/Sync    │           │                       │
│         │         │ • Network I/F    │           │                       │
│         │         └──────────────────┘           │                       │
│         │                   │                    │                       │
│         ▼                   ▼                    ▼                       │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐              │
│  │  Simulator   │    │    Fault     │    │  Analytics   │              │
│  │  (Testing)   │    │   Manager    │    │   Engine     │              │
│  └──────────────┘    └──────────────┘    └──────────────┘              │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 📡 Supported Radar Subsystems

| Subsystem | Description | Key Telemetry |
|-----------|-------------|---------------|
| **Transmitter (TX)** | High-power RF transmitter | RF Power, VSWR, Temperature, HV Voltage |
| **Receiver (RX)** | Low-noise receiver chain | Noise Figure, Gain, AGC Level |
| **Antenna & Servo** | Antenna positioning | Azimuth, Elevation, Motor Current |
| **RF Front-End** | Frequency synthesis & mixing | Phase Lock, IF Level, T/R Switch |
| **Signal Processor** | DSP & FPGA processing | CPU Load, Throughput, Latency |
| **Data Processor** | Tracking & data fusion | Active Tracks, Track Quality |
| **Power Supply (PSU)** | Power distribution & UPS | Input/Output Voltage, Battery |
| **Cooling System** | Thermal management | Coolant Temp, Flow Rate |
| **Timing & Sync** | GPS/OCXO timing | GPS Lock, Time Accuracy |
| **Network Interface** | C2 connectivity | Latency, Packet Loss |

---

## 🖥️ User Interface

### Main Window Layout

```
┌─────────────────────────────────────────────────────────────────────┐
│ [RMP Logo]  RADAR MAINTENANCE PROCESSOR  │ System: OK │ 98.5% │ 0 │
├────────────┬─────────────────────────────────────────┬──────────────┤
│            │                                         │              │
│  SUBSYSTEM │           SYSTEM CANVAS                 │   DETAILS    │
│  PALETTE   │     ┌─────┐ ┌─────┐ ┌─────┐            │   PANEL      │
│            │     │ TX  │ │ RX  │ │ ANT │            │              │
│  [TX]  ●   │     │ ●OK │ │ ●OK │ │ ●OK │            │  Health: 95% │
│  [RX]  ●   │     └─────┘ └─────┘ └─────┘            │  Faults: 0   │
│  [ANT] ●   │     ┌─────┐ ┌─────┐                    │  Telemetry   │
│  [RF]  ●   │     │ PSU │ │COOL │                    │  ...         │
│  [SP]  ●   │     │ ●OK │ │ ●OK │                    │              │
│  [DP]  ●   │     └─────┘ └─────┘                    │              │
│  [PSU] ●   │                                         │              │
│  [COOL]●   │         [Drag modules here]            │              │
│  [TIME]●   │                                         │              │
│  [NET] ●   │                                         │              │
│            │                                         │              │
├────────────┴─────────────────────────────────────────┴──────────────┤
│ Simulator: Running │ Scenario: Normal │ Active: 5/10 │       v1.0.0│
└─────────────────────────────────────────────────────────────────────┘
```

### Health States

- 🟢 **OK** - Operating normally within all parameters
- 🟡 **DEGRADED** - Reduced capability or approaching limits
- 🔴 **FAIL** - Non-operational or critical failure
- ⚫ **UNKNOWN** - Status not determined

---

## 📁 Project Structure

```
RadarMaintenanceProcessor/
├── CMakeLists.txt              # CMake build configuration
├── README.md                   # This file
│
├── include/                    # C++ Header files
│   ├── core/                   # Core system classes
│   │   ├── IRadarSubsystem.h   # Subsystem interface
│   │   ├── RadarSubsystem.h    # Base subsystem implementation
│   │   ├── SubsystemManager.h  # Central subsystem coordinator
│   │   ├── HealthDataPipeline.h# Data processing pipeline
│   │   ├── FaultManager.h      # Fault tracking & management
│   │   ├── TelemetryData.h     # Telemetry container
│   │   └── HealthStatus.h      # Status enums & types
│   │
│   ├── subsystems/             # Specific subsystem implementations
│   │   ├── TransmitterSubsystem.h
│   │   ├── ReceiverSubsystem.h
│   │   ├── AntennaServoSubsystem.h
│   │   ├── RFFrontEndSubsystem.h
│   │   ├── SignalProcessorSubsystem.h
│   │   ├── DataProcessorSubsystem.h
│   │   ├── PowerSupplySubsystem.h
│   │   ├── CoolingSubsystem.h
│   │   ├── TimingSyncSubsystem.h
│   │   └── NetworkInterfaceSubsystem.h
│   │
│   ├── simulator/              # Testing & simulation
│   │   ├── HealthSimulator.h   # Data generation
│   │   ├── FaultInjector.h     # Fault injection
│   │   └── TelemetryGenerator.h# Telemetry simulation
│   │
│   └── analytics/              # Analysis & reporting
│       ├── HealthAnalytics.h   # System analytics
│       ├── TrendAnalyzer.h     # Trend detection
│       └── UptimeTracker.h     # Availability tracking
│
├── src/                        # C++ Source files
│   ├── main.cpp                # Application entry point
│   ├── core/                   # Core implementations
│   ├── subsystems/             # Subsystem implementations
│   ├── simulator/              # Simulator implementations
│   └── analytics/              # Analytics implementations
│
├── qml/                        # QML UI files
│   ├── Main.qml                # Main application window
│   ├── components/             # Reusable UI components
│   │   ├── SystemCanvas.qml    # Central module canvas
│   │   ├── SubsystemPalette.qml# Module selection palette
│   │   ├── SubsystemModule.qml # Individual module display
│   │   ├── HealthIndicator.qml # Health status indicator
│   │   ├── TelemetryDisplay.qml# Telemetry visualization
│   │   └── FaultList.qml       # Fault listing
│   │
│   ├── panels/                 # Detail panels
│   │   ├── DetailedHealthPanel.qml
│   │   ├── AnalyticsPanel.qml
│   │   ├── SystemOverviewPanel.qml
│   │   └── FaultHistoryPanel.qml
│   │
│   ├── modules/                # Subsystem-specific modules
│   │   └── [Subsystem]Module.qml
│   │
│   └── styles/                 # Theme & styling
│       ├── RadarColors.qml     # Color palette
│       └── RadarTheme.qml      # Typography & spacing
│
├── resources/                  # Resource files
│   └── qml.qrc                 # QML resource collection
│
└── docs/                       # Documentation
    └── architecture/           # Design documents
```

---

## 🔧 Build Instructions

### Prerequisites

- **Qt 6.4+** with the following modules:
  - Qt Core
  - Qt Gui
  - Qt Qml
  - Qt Quick
  - Qt Quick Controls 2
  - Qt Charts
  - Qt Network
- **CMake 3.16+**
- **C++17** compatible compiler

### Building

```bash
# Clone the repository
git clone <repository-url>
cd RadarMaintenanceProcessor

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build . -j$(nproc)

# Run
./RadarMaintenanceProcessor
```

### Qt Installation (Ubuntu/Debian)

```bash
# Install Qt 6
sudo apt update
sudo apt install qt6-base-dev qt6-declarative-dev \
                 qt6-quickcontrols2-dev qt6-charts-dev \
                 libqt6network6 cmake build-essential
```

---

## 🎮 Usage

### Starting the Application

1. Launch the application
2. The simulator starts automatically generating health data
3. Modules on the left palette can be added to the canvas

### Configuring the System

1. **Add modules**: Click the `+` button on any subsystem in the palette
2. **Remove modules**: Hover over a module on the canvas and click `×`
3. **View details**: Click any module to open the detail panel

### Simulator Controls

- **Space**: Toggle simulator on/off
- **Scenario dropdown**: Select simulation scenario
  - Normal: Standard operation
  - Degraded: Some systems showing degradation
  - HighStress: High load conditions
  - ThermalStress: Elevated temperatures
  - PowerIssues: Power supply problems
  - PartialFailure: Component failures

---

## 🔌 API Reference

### IRadarSubsystem Interface

```cpp
class IRadarSubsystem {
public:
    virtual QString getId() const = 0;
    virtual QString getName() const = 0;
    virtual SubsystemType getType() const = 0;
    
    virtual HealthState getHealthState() const = 0;
    virtual double getHealthScore() const = 0;
    
    virtual QVariantMap getTelemetry() const = 0;
    virtual QVariantList getFaults() const = 0;
    
    virtual void updateData(const QVariantMap& data) = 0;
    virtual void processHealthData() = 0;
};
```

### Extending with New Subsystems

1. Create header in `include/subsystems/`
2. Inherit from `RadarSubsystem`
3. Override `initializeTelemetryParameters()` to define telemetry
4. Override `computeHealthState()` and `computeHealthScore()`
5. Register in `main.cpp`

---

## 🧪 Testing

### Unit Tests

```bash
cd build
ctest --output-on-failure
```

### Simulation Scenarios

The built-in simulator supports various test scenarios:

- **Normal**: Validates baseline operation
- **PartialFailure**: Tests fault detection and display
- **HighStress**: Tests system under load
- **CascadingFailure**: Tests multi-subsystem fault handling

---

## 🚀 Future Roadmap

- [ ] Network-based health input (UDP/TCP)
- [ ] Redundant subsystem support
- [ ] Multi-radar scaling
- [ ] Historical data persistence
- [ ] Advanced trend visualization with charts
- [ ] Alert notifications
- [ ] Configuration import/export
- [ ] Secure authentication

---

## 📄 License

Proprietary - Defence Application

---

## 👥 Authors

Radar Systems Engineering Team

---

## 🔗 Related Projects

- Radar Signal Processing Library
- C2 Integration Framework
- Defence HMI Standards Toolkit
