#ifndef POWERSUPPLYSUBSYSTEM_H
#define POWERSUPPLYSUBSYSTEM_H

#include "core/RadarSubsystem.h"

namespace RadarRMP {

/**
 * @brief Power Supply Unit (PSU) subsystem implementation
 * 
 * Monitors and reports health of the power supply system:
 * - Input/output voltages
 * - Current draw
 * - Power factor
 * - UPS status
 * - Battery health
 */
class PowerSupplySubsystem : public RadarSubsystem {
    Q_OBJECT
    
    Q_PROPERTY(double inputVoltage READ getInputVoltage NOTIFY telemetryChanged)
    Q_PROPERTY(double outputVoltage READ getOutputVoltage NOTIFY telemetryChanged)
    Q_PROPERTY(double current READ getCurrent NOTIFY telemetryChanged)
    Q_PROPERTY(double power READ getPower NOTIFY telemetryChanged)
    Q_PROPERTY(double temperature READ getTemperature NOTIFY telemetryChanged)
    Q_PROPERTY(double batteryLevel READ getBatteryLevel NOTIFY telemetryChanged)
    Q_PROPERTY(bool onBattery READ isOnBattery NOTIFY telemetryChanged)
    Q_PROPERTY(double efficiency READ getEfficiency NOTIFY telemetryChanged)
    
public:
    explicit PowerSupplySubsystem(const QString& id, const QString& name = "Power Supply",
                                  QObject* parent = nullptr);
    ~PowerSupplySubsystem() override = default;
    
    double getInputVoltage() const;   // VAC
    double getOutputVoltage() const;  // VDC
    double getCurrent() const;        // Amps
    double getPower() const;          // Watts
    double getTemperature() const;    // Celsius
    double getBatteryLevel() const;   // Percentage
    bool isOnBattery() const;
    double getEfficiency() const;     // Percentage
    double getPowerFactor() const;
    QString getPsuMode() const;       // "NORMAL", "BATTERY", "BYPASS"
    
    // PSU fault codes
    static constexpr const char* FAULT_INPUT_LOW = "PSU-001";
    static constexpr const char* FAULT_INPUT_HIGH = "PSU-002";
    static constexpr const char* FAULT_OUTPUT_LOW = "PSU-003";
    static constexpr const char* FAULT_OUTPUT_HIGH = "PSU-004";
    static constexpr const char* FAULT_OVERCURRENT = "PSU-005";
    static constexpr const char* FAULT_OVERTEMP = "PSU-006";
    static constexpr const char* FAULT_BATTERY_LOW = "PSU-007";
    static constexpr const char* FAULT_BATTERY_FAIL = "PSU-008";
    static constexpr const char* FAULT_UPS_FAIL = "PSU-009";
    
protected:
    void initializeTelemetryParameters() override;
    HealthState computeHealthState() const override;
    double computeHealthScore() const override;
    QString computeStatusMessage() const override;
    void onDataUpdate(const QVariantMap& data) override;
    
private:
    static constexpr double INPUT_VOLTAGE_LOW_WARNING = 200.0;
    static constexpr double INPUT_VOLTAGE_LOW_CRITICAL = 180.0;
    static constexpr double INPUT_VOLTAGE_HIGH_WARNING = 250.0;
    static constexpr double INPUT_VOLTAGE_HIGH_CRITICAL = 270.0;
    static constexpr double BATTERY_WARNING = 30.0;          // Percentage
    static constexpr double BATTERY_CRITICAL = 10.0;
    static constexpr double TEMP_WARNING = 50.0;
    static constexpr double TEMP_CRITICAL = 65.0;
};

} // namespace RadarRMP

#endif // POWERSUPPLYSUBSYSTEM_H
