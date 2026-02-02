#ifndef RFFRONTENDSUBSYSTEM_H
#define RFFRONTENDSUBSYSTEM_H

#include "core/RadarSubsystem.h"

namespace RadarRMP {

/**
 * @brief RF Front-End subsystem implementation
 * 
 * Monitors and reports health of the RF front-end components:
 * - Frequency synthesizer
 * - Mixers
 * - Filters
 * - T/R switches
 * - Phase/amplitude calibration
 */
class RFFrontEndSubsystem : public RadarSubsystem {
    Q_OBJECT
    
    Q_PROPERTY(double frequency READ getFrequency NOTIFY telemetryChanged)
    Q_PROPERTY(double phaseLock READ getPhaseLock NOTIFY telemetryChanged)
    Q_PROPERTY(double ifLevel READ getIfLevel NOTIFY telemetryChanged)
    Q_PROPERTY(double loLevel READ getLoLevel NOTIFY telemetryChanged)
    Q_PROPERTY(double temperature READ getTemperature NOTIFY telemetryChanged)
    Q_PROPERTY(bool trSwitchOk READ isTrSwitchOk NOTIFY telemetryChanged)
    
public:
    explicit RFFrontEndSubsystem(const QString& id, const QString& name = "RF Front-End",
                                 QObject* parent = nullptr);
    ~RFFrontEndSubsystem() override = default;
    
    double getFrequency() const;      // GHz
    double getPhaseLock() const;      // Lock indicator (0-1)
    double getIfLevel() const;        // dBm
    double getLoLevel() const;        // dBm
    double getTemperature() const;    // Celsius
    bool isTrSwitchOk() const;
    double getPhaseError() const;     // Degrees
    double getAmplitudeError() const; // dB
    
    // RF Front-End fault codes
    static constexpr const char* FAULT_PLL_UNLOCK = "RF-001";
    static constexpr const char* FAULT_IF_LEVEL = "RF-002";
    static constexpr const char* FAULT_LO_LEVEL = "RF-003";
    static constexpr const char* FAULT_TR_SWITCH = "RF-004";
    static constexpr const char* FAULT_PHASE_CAL = "RF-005";
    static constexpr const char* FAULT_AMP_CAL = "RF-006";
    static constexpr const char* FAULT_OVERTEMP = "RF-007";
    
protected:
    void initializeTelemetryParameters() override;
    HealthState computeHealthState() const override;
    double computeHealthScore() const override;
    QString computeStatusMessage() const override;
    void onDataUpdate(const QVariantMap& data) override;
    
private:
    static constexpr double PHASE_LOCK_WARNING = 0.8;
    static constexpr double PHASE_LOCK_CRITICAL = 0.5;
    static constexpr double TEMP_WARNING = 55.0;
    static constexpr double TEMP_CRITICAL = 70.0;
};

} // namespace RadarRMP

#endif // RFFRONTENDSUBSYSTEM_H
