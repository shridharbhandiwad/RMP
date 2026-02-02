#ifndef ANTENNASERVOSUBSYSTEM_H
#define ANTENNASERVOSUBSYSTEM_H

#include "core/RadarSubsystem.h"

namespace RadarRMP {

/**
 * @brief Antenna and Servo subsystem implementation
 * 
 * Monitors and reports health of the antenna positioner and servo system:
 * - Azimuth and elevation positions
 * - Rotation rate
 * - Motor current/temperature
 * - Position accuracy
 * - Limit switch status
 */
class AntennaServoSubsystem : public RadarSubsystem {
    Q_OBJECT
    
    Q_PROPERTY(double azimuth READ getAzimuth NOTIFY telemetryChanged)
    Q_PROPERTY(double elevation READ getElevation NOTIFY telemetryChanged)
    Q_PROPERTY(double rotationRate READ getRotationRate NOTIFY telemetryChanged)
    Q_PROPERTY(double motorCurrent READ getMotorCurrent NOTIFY telemetryChanged)
    Q_PROPERTY(double motorTemperature READ getMotorTemperature NOTIFY telemetryChanged)
    Q_PROPERTY(double positionError READ getPositionError NOTIFY telemetryChanged)
    Q_PROPERTY(QString scanMode READ getScanMode NOTIFY telemetryChanged)
    
public:
    explicit AntennaServoSubsystem(const QString& id, const QString& name = "Antenna & Servo",
                                   QObject* parent = nullptr);
    ~AntennaServoSubsystem() override = default;
    
    double getAzimuth() const;          // Degrees
    double getElevation() const;        // Degrees
    double getRotationRate() const;     // RPM or deg/sec
    double getMotorCurrent() const;     // Amps
    double getMotorTemperature() const; // Celsius
    double getPositionError() const;    // Degrees
    QString getScanMode() const;        // "SEARCH", "TRACK", "STARE"
    bool isAzLimitReached() const;
    bool isElLimitReached() const;
    
    // Antenna fault codes
    static constexpr const char* FAULT_MOTOR_OVERCURRENT = "ANT-001";
    static constexpr const char* FAULT_MOTOR_OVERTEMP = "ANT-002";
    static constexpr const char* FAULT_POSITION_ERROR = "ANT-003";
    static constexpr const char* FAULT_SERVO_FAIL = "ANT-004";
    static constexpr const char* FAULT_AZ_LIMIT = "ANT-005";
    static constexpr const char* FAULT_EL_LIMIT = "ANT-006";
    static constexpr const char* FAULT_ENCODER_FAIL = "ANT-007";
    static constexpr const char* FAULT_STALL = "ANT-008";
    
protected:
    void initializeTelemetryParameters() override;
    HealthState computeHealthState() const override;
    double computeHealthScore() const override;
    QString computeStatusMessage() const override;
    void onDataUpdate(const QVariantMap& data) override;
    
private:
    static constexpr double CURRENT_WARNING = 8.0;       // Amps
    static constexpr double CURRENT_CRITICAL = 12.0;
    static constexpr double MOTOR_TEMP_WARNING = 65.0;   // Celsius
    static constexpr double MOTOR_TEMP_CRITICAL = 85.0;
    static constexpr double POSITION_ERROR_WARNING = 0.5; // Degrees
    static constexpr double POSITION_ERROR_CRITICAL = 1.0;
};

} // namespace RadarRMP

#endif // ANTENNASERVOSUBSYSTEM_H
