#ifndef TRANSMITTERSUBSYSTEM_H
#define TRANSMITTERSUBSYSTEM_H

#include "core/RadarSubsystem.h"

namespace RadarRMP {

/**
 * @brief Transmitter (TX) subsystem implementation
 * 
 * Monitors and reports health of the radar transmitter including:
 * - RF output power
 * - Pulse characteristics
 * - VSWR (Voltage Standing Wave Ratio)
 * - Temperature
 * - High voltage status
 * - Modulator health
 */
class TransmitterSubsystem : public RadarSubsystem {
    Q_OBJECT
    
    // TX-specific properties
    Q_PROPERTY(double rfPower READ getRfPower NOTIFY telemetryChanged)
    Q_PROPERTY(double vswr READ getVswr NOTIFY telemetryChanged)
    Q_PROPERTY(double temperature READ getTemperature NOTIFY telemetryChanged)
    Q_PROPERTY(double dutyCycle READ getDutyCycle NOTIFY telemetryChanged)
    Q_PROPERTY(double hvVoltage READ getHvVoltage NOTIFY telemetryChanged)
    Q_PROPERTY(bool hvEnabled READ isHvEnabled NOTIFY telemetryChanged)
    Q_PROPERTY(QString txMode READ getTxMode NOTIFY telemetryChanged)
    
public:
    explicit TransmitterSubsystem(const QString& id, const QString& name = "Transmitter",
                                  QObject* parent = nullptr);
    ~TransmitterSubsystem() override = default;
    
    // TX-specific getters
    double getRfPower() const;      // Watts
    double getVswr() const;         // Ratio
    double getTemperature() const;  // Celsius
    double getDutyCycle() const;    // Percentage
    double getHvVoltage() const;    // kV
    bool isHvEnabled() const;
    QString getTxMode() const;
    double getPulseWidth() const;   // microseconds
    double getPrf() const;          // Hz (Pulse Repetition Frequency)
    
    // TX fault codes
    static constexpr const char* FAULT_RF_POWER_LOW = "TX-001";
    static constexpr const char* FAULT_RF_POWER_HIGH = "TX-002";
    static constexpr const char* FAULT_VSWR_HIGH = "TX-003";
    static constexpr const char* FAULT_OVERTEMP = "TX-004";
    static constexpr const char* FAULT_HV_FAIL = "TX-005";
    static constexpr const char* FAULT_MODULATOR = "TX-006";
    static constexpr const char* FAULT_ARC_DETECT = "TX-007";
    static constexpr const char* FAULT_INTERLOCK = "TX-008";
    
protected:
    void initializeTelemetryParameters() override;
    HealthState computeHealthState() const override;
    double computeHealthScore() const override;
    QString computeStatusMessage() const override;
    void onDataUpdate(const QVariantMap& data) override;
    
private:
    // Threshold values
    static constexpr double RF_POWER_WARNING_LOW = 80.0;   // % of nominal
    static constexpr double RF_POWER_CRITICAL_LOW = 50.0;
    static constexpr double VSWR_WARNING = 1.5;
    static constexpr double VSWR_CRITICAL = 2.0;
    static constexpr double TEMP_WARNING = 60.0;           // Celsius
    static constexpr double TEMP_CRITICAL = 80.0;
    static constexpr double HV_WARNING_LOW = 90.0;         // % of nominal
    static constexpr double HV_CRITICAL_LOW = 80.0;
};

} // namespace RadarRMP

#endif // TRANSMITTERSUBSYSTEM_H
