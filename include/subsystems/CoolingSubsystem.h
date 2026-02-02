#ifndef COOLINGSUBSYSTEM_H
#define COOLINGSUBSYSTEM_H

#include "core/RadarSubsystem.h"

namespace RadarRMP {

/**
 * @brief Cooling / Thermal Management subsystem implementation
 * 
 * Monitors and reports health of the thermal management system:
 * - Coolant temperature and flow
 * - Fan speeds
 * - Heat exchanger efficiency
 * - Ambient temperature
 * - Thermal zones
 */
class CoolingSubsystem : public RadarSubsystem {
    Q_OBJECT
    
    Q_PROPERTY(double coolantTemp READ getCoolantTemp NOTIFY telemetryChanged)
    Q_PROPERTY(double coolantFlow READ getCoolantFlow NOTIFY telemetryChanged)
    Q_PROPERTY(double ambientTemp READ getAmbientTemp NOTIFY telemetryChanged)
    Q_PROPERTY(double fanSpeed READ getFanSpeed NOTIFY telemetryChanged)
    Q_PROPERTY(double heatLoad READ getHeatLoad NOTIFY telemetryChanged)
    Q_PROPERTY(double efficiency READ getEfficiency NOTIFY telemetryChanged)
    Q_PROPERTY(QString coolingMode READ getCoolingMode NOTIFY telemetryChanged)
    
public:
    explicit CoolingSubsystem(const QString& id, const QString& name = "Cooling System",
                              QObject* parent = nullptr);
    ~CoolingSubsystem() override = default;
    
    double getCoolantTemp() const;     // Celsius
    double getCoolantFlow() const;     // L/min
    double getAmbientTemp() const;     // Celsius
    double getFanSpeed() const;        // RPM or percentage
    double getHeatLoad() const;        // kW
    double getEfficiency() const;      // Percentage
    QString getCoolingMode() const;    // "AUTO", "HIGH", "LOW", "EMERGENCY"
    double getCompressorPressure() const;
    bool isCompressorRunning() const;
    
    // Cooling fault codes
    static constexpr const char* FAULT_COOLANT_TEMP_HIGH = "COOL-001";
    static constexpr const char* FAULT_COOLANT_FLOW_LOW = "COOL-002";
    static constexpr const char* FAULT_FAN_FAIL = "COOL-003";
    static constexpr const char* FAULT_COMPRESSOR_FAIL = "COOL-004";
    static constexpr const char* FAULT_AMBIENT_HIGH = "COOL-005";
    static constexpr const char* FAULT_EFFICIENCY_LOW = "COOL-006";
    static constexpr const char* FAULT_COOLANT_LOW = "COOL-007";
    static constexpr const char* FAULT_HEAT_EXCHANGER = "COOL-008";
    
protected:
    void initializeTelemetryParameters() override;
    HealthState computeHealthState() const override;
    double computeHealthScore() const override;
    QString computeStatusMessage() const override;
    void onDataUpdate(const QVariantMap& data) override;
    
private:
    static constexpr double COOLANT_TEMP_WARNING = 45.0;     // Celsius
    static constexpr double COOLANT_TEMP_CRITICAL = 55.0;
    static constexpr double FLOW_WARNING_LOW = 70.0;         // % of nominal
    static constexpr double FLOW_CRITICAL_LOW = 50.0;
    static constexpr double AMBIENT_WARNING = 40.0;
    static constexpr double AMBIENT_CRITICAL = 50.0;
    static constexpr double EFFICIENCY_WARNING = 70.0;       // Percentage
    static constexpr double EFFICIENCY_CRITICAL = 50.0;
};

} // namespace RadarRMP

#endif // COOLINGSUBSYSTEM_H
