#ifndef RECEIVERSUBSYSTEM_H
#define RECEIVERSUBSYSTEM_H

#include "core/RadarSubsystem.h"

namespace RadarRMP {

/**
 * @brief Receiver (RX) subsystem implementation
 * 
 * Monitors and reports health of the radar receiver including:
 * - Noise figure
 * - Gain levels
 * - AGC status
 * - LNA health
 * - ADC performance
 * - Signal quality metrics
 */
class ReceiverSubsystem : public RadarSubsystem {
    Q_OBJECT
    
    Q_PROPERTY(double noiseFigure READ getNoiseFigure NOTIFY telemetryChanged)
    Q_PROPERTY(double gain READ getGain NOTIFY telemetryChanged)
    Q_PROPERTY(double agcLevel READ getAgcLevel NOTIFY telemetryChanged)
    Q_PROPERTY(double temperature READ getTemperature NOTIFY telemetryChanged)
    Q_PROPERTY(double signalLevel READ getSignalLevel NOTIFY telemetryChanged)
    Q_PROPERTY(bool lnaEnabled READ isLnaEnabled NOTIFY telemetryChanged)
    
public:
    explicit ReceiverSubsystem(const QString& id, const QString& name = "Receiver",
                               QObject* parent = nullptr);
    ~ReceiverSubsystem() override = default;
    
    double getNoiseFigure() const;    // dB
    double getGain() const;           // dB
    double getAgcLevel() const;       // dB
    double getTemperature() const;    // Celsius
    double getSignalLevel() const;    // dBm
    bool isLnaEnabled() const;
    double getDynamicRange() const;   // dB
    double getSensitivity() const;    // dBm
    
    // RX fault codes
    static constexpr const char* FAULT_NOISE_FIGURE_HIGH = "RX-001";
    static constexpr const char* FAULT_GAIN_LOW = "RX-002";
    static constexpr const char* FAULT_LNA_FAIL = "RX-003";
    static constexpr const char* FAULT_AGC_FAIL = "RX-004";
    static constexpr const char* FAULT_ADC_ERROR = "RX-005";
    static constexpr const char* FAULT_OVERTEMP = "RX-006";
    static constexpr const char* FAULT_SATURATION = "RX-007";
    
protected:
    void initializeTelemetryParameters() override;
    HealthState computeHealthState() const override;
    double computeHealthScore() const override;
    QString computeStatusMessage() const override;
    void onDataUpdate(const QVariantMap& data) override;
    
private:
    static constexpr double NOISE_FIGURE_WARNING = 4.0;    // dB
    static constexpr double NOISE_FIGURE_CRITICAL = 6.0;
    static constexpr double GAIN_WARNING_LOW = 25.0;       // dB
    static constexpr double GAIN_CRITICAL_LOW = 20.0;
    static constexpr double TEMP_WARNING = 55.0;
    static constexpr double TEMP_CRITICAL = 70.0;
};

} // namespace RadarRMP

#endif // RECEIVERSUBSYSTEM_H
