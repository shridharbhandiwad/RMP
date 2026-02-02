#ifndef IRADARSUBSYSTEM_H
#define IRADARSUBSYSTEM_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include "HealthStatus.h"

namespace RadarRMP {

/**
 * @brief Interface for all radar subsystems
 * 
 * This abstract interface defines the contract that all radar subsystems
 * must implement. It provides a uniform API for:
 * - Health status retrieval
 * - Telemetry data access
 * - Fault management
 * - Subsystem identification
 * 
 * The interface is designed to be Qt-friendly with QVariant types for
 * seamless integration with QML.
 */
class IRadarSubsystem {
public:
    virtual ~IRadarSubsystem() = default;
    
    // ===== Identification =====
    
    /**
     * @brief Get unique subsystem identifier
     * @return Unique string ID (e.g., "TX-001")
     */
    virtual QString getId() const = 0;
    
    /**
     * @brief Get human-readable subsystem name
     * @return Display name (e.g., "Main Transmitter")
     */
    virtual QString getName() const = 0;
    
    /**
     * @brief Get subsystem type
     * @return SubsystemType enumeration value
     */
    virtual SubsystemType getType() const = 0;
    
    /**
     * @brief Get subsystem description
     * @return Detailed description of the subsystem
     */
    virtual QString getDescription() const = 0;
    
    // ===== Health Status =====
    
    /**
     * @brief Get current health state
     * @return HealthState (OK, DEGRADED, FAIL, UNKNOWN)
     */
    virtual HealthState getHealthState() const = 0;
    
    /**
     * @brief Get health state as string for QML
     * @return String representation of health state
     */
    virtual QString getHealthStateString() const = 0;
    
    /**
     * @brief Get detailed health status snapshot
     * @return Complete health information
     */
    virtual HealthSnapshot getHealthSnapshot() const = 0;
    
    /**
     * @brief Get health score (0-100)
     * @return Numeric health score
     */
    virtual double getHealthScore() const = 0;
    
    /**
     * @brief Get status message
     * @return Human-readable status message
     */
    virtual QString getStatusMessage() const = 0;
    
    // ===== Telemetry =====
    
    /**
     * @brief Get all telemetry parameters
     * @return Map of parameter name to value
     */
    virtual QVariantMap getTelemetry() const = 0;
    
    /**
     * @brief Get specific telemetry parameter
     * @param paramName Parameter name
     * @return Parameter value or invalid QVariant if not found
     */
    virtual QVariant getTelemetryValue(const QString& paramName) const = 0;
    
    /**
     * @brief Get list of available telemetry parameter names
     * @return List of parameter names
     */
    virtual QStringList getTelemetryParameters() const = 0;
    
    /**
     * @brief Get telemetry parameter metadata (units, limits, etc.)
     * @param paramName Parameter name
     * @return Metadata map
     */
    virtual QVariantMap getTelemetryMetadata(const QString& paramName) const = 0;
    
    // ===== Faults =====
    
    /**
     * @brief Get all active faults
     * @return List of active fault codes
     */
    virtual QVariantList getFaults() const = 0;
    
    /**
     * @brief Get fault history
     * @param maxCount Maximum number of faults to return
     * @return Historical fault list
     */
    virtual QVariantList getFaultHistory(int maxCount = 100) const = 0;
    
    /**
     * @brief Check if subsystem has any active faults
     * @return True if faults are present
     */
    virtual bool hasFaults() const = 0;
    
    /**
     * @brief Get count of active faults
     * @return Number of active faults
     */
    virtual int getFaultCount() const = 0;
    
    /**
     * @brief Clear a specific fault (if clearable)
     * @param faultCode Fault code to clear
     * @return True if fault was cleared
     */
    virtual bool clearFault(const QString& faultCode) = 0;
    
    /**
     * @brief Clear all clearable faults
     * @return Number of faults cleared
     */
    virtual int clearAllFaults() = 0;
    
    // ===== Control =====
    
    /**
     * @brief Check if subsystem is enabled
     * @return True if enabled
     */
    virtual bool isEnabled() const = 0;
    
    /**
     * @brief Enable or disable subsystem
     * @param enabled Desired state
     */
    virtual void setEnabled(bool enabled) = 0;
    
    /**
     * @brief Reset subsystem to initial state
     */
    virtual void reset() = 0;
    
    /**
     * @brief Perform self-test
     * @return True if self-test passed
     */
    virtual bool runSelfTest() = 0;
    
    // ===== Update =====
    
    /**
     * @brief Update subsystem with new data
     * @param data New telemetry/status data
     */
    virtual void updateData(const QVariantMap& data) = 0;
    
    /**
     * @brief Process health data through pipeline
     */
    virtual void processHealthData() = 0;
};

} // namespace RadarRMP

#endif // IRADARSUBSYSTEM_H
