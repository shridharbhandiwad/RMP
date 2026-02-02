#ifndef HEALTHSTATUS_H
#define HEALTHSTATUS_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariantMap>

namespace RadarRMP {

/**
 * @brief Health state enumeration for radar subsystems
 * 
 * Represents the three-state health model used in defence radar systems:
 * - OK: Subsystem operating within all normal parameters
 * - DEGRADED: Subsystem operating with reduced capability or approaching limits
 * - FAIL: Subsystem has failed or is non-operational
 */
enum class HealthState {
    OK = 0,
    DEGRADED = 1,
    FAIL = 2,
    UNKNOWN = 3
};

/**
 * @brief Fault severity levels
 */
enum class FaultSeverity {
    INFO = 0,
    WARNING = 1,
    CRITICAL = 2,
    FATAL = 3
};

/**
 * @brief Subsystem type enumeration
 */
enum class SubsystemType {
    Transmitter,
    Receiver,
    AntennaServo,
    RFFrontEnd,
    SignalProcessor,
    DataProcessor,
    PowerSupply,
    Cooling,
    TimingSync,
    NetworkInterface
};

/**
 * @brief Fault code structure
 */
struct FaultCode {
    QString code;           // Unique fault identifier (e.g., "TX-001")
    QString description;    // Human-readable description
    FaultSeverity severity; // Severity level
    QDateTime timestamp;    // When the fault occurred
    QString subsystemId;    // Which subsystem reported the fault
    bool active;            // Is the fault currently active
    QVariantMap metadata;   // Additional fault-specific data
    
    FaultCode() : severity(FaultSeverity::INFO), active(false) {}
    
    FaultCode(const QString& c, const QString& desc, FaultSeverity sev, 
              const QString& subsys)
        : code(c), description(desc), severity(sev), 
          timestamp(QDateTime::currentDateTime()), subsystemId(subsys), 
          active(true) {}
};

/**
 * @brief Health status snapshot for a subsystem
 */
struct HealthSnapshot {
    HealthState state;
    QDateTime timestamp;
    QVariantMap telemetry;
    QList<FaultCode> activeFaults;
    double healthScore;     // 0.0 to 100.0
    QString statusMessage;
    
    HealthSnapshot() 
        : state(HealthState::UNKNOWN), 
          timestamp(QDateTime::currentDateTime()),
          healthScore(100.0) {}
};

// Qt Meta-type declarations for use in signals/slots
inline QString healthStateToString(HealthState state) {
    switch (state) {
        case HealthState::OK: return "OK";
        case HealthState::DEGRADED: return "DEGRADED";
        case HealthState::FAIL: return "FAIL";
        default: return "UNKNOWN";
    }
}

inline QString faultSeverityToString(FaultSeverity severity) {
    switch (severity) {
        case FaultSeverity::INFO: return "INFO";
        case FaultSeverity::WARNING: return "WARNING";
        case FaultSeverity::CRITICAL: return "CRITICAL";
        case FaultSeverity::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

inline QString subsystemTypeToString(SubsystemType type) {
    switch (type) {
        case SubsystemType::Transmitter: return "Transmitter";
        case SubsystemType::Receiver: return "Receiver";
        case SubsystemType::AntennaServo: return "Antenna & Servo";
        case SubsystemType::RFFrontEnd: return "RF Front-End";
        case SubsystemType::SignalProcessor: return "Signal Processor";
        case SubsystemType::DataProcessor: return "Data Processor";
        case SubsystemType::PowerSupply: return "Power Supply";
        case SubsystemType::Cooling: return "Cooling System";
        case SubsystemType::TimingSync: return "Timing & Sync";
        case SubsystemType::NetworkInterface: return "Network Interface";
        default: return "Unknown";
    }
}

inline QString subsystemTypeToIcon(SubsystemType type) {
    switch (type) {
        case SubsystemType::Transmitter: return "📡";
        case SubsystemType::Receiver: return "📻";
        case SubsystemType::AntennaServo: return "🎯";
        case SubsystemType::RFFrontEnd: return "📶";
        case SubsystemType::SignalProcessor: return "🔬";
        case SubsystemType::DataProcessor: return "💻";
        case SubsystemType::PowerSupply: return "⚡";
        case SubsystemType::Cooling: return "❄️";
        case SubsystemType::TimingSync: return "⏱️";
        case SubsystemType::NetworkInterface: return "🌐";
        default: return "❓";
    }
}

} // namespace RadarRMP

Q_DECLARE_METATYPE(RadarRMP::HealthState)
Q_DECLARE_METATYPE(RadarRMP::FaultSeverity)
Q_DECLARE_METATYPE(RadarRMP::SubsystemType)

#endif // HEALTHSTATUS_H
