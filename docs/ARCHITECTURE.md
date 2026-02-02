# Radar Maintenance Processor - System Architecture

## 1. High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                             PRESENTATION LAYER                               │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                          QML/Qt Quick UI                              │   │
│  │  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌─────────────┐│   │
│  │  │   System     │ │  Subsystem   │ │   Detail     │ │  Analytics  ││   │
│  │  │   Canvas     │ │   Palette    │ │   Panels     │ │   Views     ││   │
│  │  └──────────────┘ └──────────────┘ └──────────────┘ └─────────────┘│   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└───────────────────────────────────┬─────────────────────────────────────────┘
                                    │ Qt Signals/Slots & Property Binding
┌───────────────────────────────────▼─────────────────────────────────────────┐
│                              BUSINESS LAYER                                  │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                        SubsystemManager                              │   │
│  │  - Subsystem lifecycle management                                    │   │
│  │  - Active subsystem coordination                                     │   │
│  │  - System-wide health computation                                    │   │
│  └───────────────────────────────┬─────────────────────────────────────┘   │
│                                  │                                          │
│  ┌───────────────┐ ┌─────────────▼───────────┐ ┌─────────────────────────┐ │
│  │ FaultManager  │ │   RadarSubsystem (x10)  │ │   HealthDataPipeline    │ │
│  │ - Fault track │ │   - Health state        │ │   - Data validation     │ │
│  │ - MTBF calc   │ │   - Telemetry           │ │   - Status computation  │ │
│  │ - Statistics  │ │   - Fault management    │ │   - Threshold checking  │ │
│  └───────────────┘ └─────────────────────────┘ └─────────────────────────┘ │
└───────────────────────────────────┬─────────────────────────────────────────┘
                                    │
┌───────────────────────────────────▼─────────────────────────────────────────┐
│                             ANALYTICS LAYER                                  │
│  ┌──────────────────┐ ┌──────────────────┐ ┌──────────────────┐            │
│  │  HealthAnalytics │ │   TrendAnalyzer  │ │   UptimeTracker  │            │
│  │  - Summaries     │ │   - Regression   │ │   - Availability │            │
│  │  - Reports       │ │   - Anomalies    │ │   - State track  │            │
│  │  - History       │ │   - Predictions  │ │   - Reports      │            │
│  └──────────────────┘ └──────────────────┘ └──────────────────┘            │
└───────────────────────────────────┬─────────────────────────────────────────┘
                                    │
┌───────────────────────────────────▼─────────────────────────────────────────┐
│                            SIMULATION LAYER                                  │
│  ┌──────────────────┐ ┌──────────────────┐ ┌──────────────────┐            │
│  │  HealthSimulator │ │   FaultInjector  │ │TelemetryGenerator│            │
│  │  - Scenarios     │ │   - Controlled   │ │   - Noise        │            │
│  │  - Data gen      │ │   - Scheduled    │ │   - Trends       │            │
│  │  - State vary    │ │   - Scenarios    │ │   - Anomalies    │            │
│  └──────────────────┘ └──────────────────┘ └──────────────────┘            │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 2. Class Hierarchy

```
IRadarSubsystem (Interface)
    │
    └── RadarSubsystem (Base Class)
            │
            ├── TransmitterSubsystem
            ├── ReceiverSubsystem
            ├── AntennaServoSubsystem
            ├── RFFrontEndSubsystem
            ├── SignalProcessorSubsystem
            ├── DataProcessorSubsystem
            ├── PowerSupplySubsystem
            ├── CoolingSubsystem
            ├── TimingSyncSubsystem
            └── NetworkInterfaceSubsystem
```

## 3. Data Flow

```
┌────────────┐     ┌──────────────┐     ┌────────────────┐     ┌──────────┐
│  External  │────►│   Health     │────►│   Subsystem    │────►│   QML    │
│   Data     │     │   Pipeline   │     │   (Telemetry)  │     │    UI    │
└────────────┘     └──────────────┘     └────────────────┘     └──────────┘
                         │                      │
                         │ Validation           │ State Change
                         ▼                      ▼
                   ┌──────────┐          ┌──────────────┐
                   │  Fault   │          │   Analytics  │
                   │ Manager  │          │    Engine    │
                   └──────────┘          └──────────────┘
```

## 4. Signal/Slot Communication

```cpp
// Subsystem Health Changes
RadarSubsystem::healthChanged()
    └──► SubsystemManager::onSubsystemHealthChanged()
            └──► SubsystemManager::computeSystemHealth()
                    └──► SubsystemManager::systemHealthChanged()
                            └──► QML Property Binding Updates

// Fault Propagation
RadarSubsystem::faultOccurred(code, description)
    └──► SubsystemManager::onSubsystemFaultOccurred()
            └──► FaultManager::registerFault()
                    └──► FaultManager::faultsChanged()
                            └──► QML UI Updates

// Telemetry Updates
HealthSimulator::onUpdateTick()
    └──► RadarSubsystem::updateData(telemetry)
            └──► RadarSubsystem::processHealthData()
                    └──► RadarSubsystem::telemetryChanged()
                            └──► TrendAnalyzer::addDataPoints()
                            └──► QML Property Binding Updates
```

## 5. State Machine

```
                        ┌──────────┐
                        │  UNKNOWN │
                        └────┬─────┘
                             │ Initialize
                        ┌────▼─────┐
            ┌───────────│    OK    │◄──────────┐
            │           └────┬─────┘           │
            │                │                 │
       Warning          Threshold          Recovery
            │            Exceeded              │
            │                │                 │
       ┌────▼─────┐     ┌────▼─────┐          │
       │ DEGRADED │◄───►│   FAIL   │──────────┘
       └──────────┘     └──────────┘
```

## 6. Component Responsibilities

### SubsystemManager
- Central coordinator for all subsystems
- Maintains active/inactive subsystem lists
- Computes system-wide health metrics
- Routes telemetry and fault notifications

### RadarSubsystem (Base)
- Common telemetry storage via TelemetryData
- Fault list management
- Health state computation framework
- QML property exposure via Q_PROPERTY

### HealthDataPipeline
- Input data validation
- Threshold checking
- Derived metric computation
- Fault detection from telemetry

### FaultManager
- Centralized fault registry
- Fault history with timestamps
- MTBF (Mean Time Between Failures) calculation
- Severity-based categorization

### HealthSimulator
- Realistic telemetry generation
- Scenario-based simulation
- Random fault injection
- State variation over time

## 7. QML Component Architecture

```
Main.qml
    │
    ├── SubsystemPalette.qml
    │       └── SubsystemPaletteItem (component)
    │
    ├── SystemCanvas.qml
    │       └── SubsystemModule.qml
    │               ├── HealthIndicator.qml
    │               └── MetricItem (component)
    │
    └── DetailedHealthPanel.qml
            ├── StatusTab (component)
            ├── TelemetryDisplay.qml
            ├── FaultList.qml
            └── TrendsTab (component)
```

## 8. Extensibility Points

### Adding New Subsystems
1. Create `NewSubsystem.h/cpp` inheriting `RadarSubsystem`
2. Add to `SubsystemType` enum
3. Implement `initializeTelemetryParameters()`
4. Override health computation methods
5. Register in `main.cpp`
6. (Optional) Create custom QML module

### Adding New Telemetry Parameters
1. Define `TelemetryParameter` with thresholds
2. Add in `initializeTelemetryParameters()`
3. Create getter method
4. Update `onDataUpdate()` for fault checking

### Adding New Fault Types
1. Define static fault code constant
2. Add detection logic in `onDataUpdate()`
3. Map to appropriate severity

## 9. Thread Safety

- `RadarSubsystem` uses `QMutex` for state protection
- Health pipeline can operate in background thread
- All QML updates via Qt event loop
- Signals/slots handle thread boundaries

## 10. Performance Considerations

- Update interval configurable (default: 1 second)
- Lazy health score computation
- Efficient QVariantMap for telemetry
- Prune historical data automatically
- Use Qt property bindings vs polling
