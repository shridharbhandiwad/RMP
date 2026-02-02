#ifndef TIMINGSYNCSUBSYSTEM_H
#define TIMINGSYNCSUBSYSTEM_H

#include "core/RadarSubsystem.h"

namespace RadarRMP {

/**
 * @brief Timing & Synchronization (GPS/OCXO) subsystem implementation
 * 
 * Monitors and reports health of the timing system:
 * - GPS lock status
 * - OCXO stability
 * - Time accuracy
 * - PPS (Pulse Per Second) quality
 * - Time synchronization status
 */
class TimingSyncSubsystem : public RadarSubsystem {
    Q_OBJECT
    
    Q_PROPERTY(bool gpsLocked READ isGpsLocked NOTIFY telemetryChanged)
    Q_PROPERTY(int satelliteCount READ getSatelliteCount NOTIFY telemetryChanged)
    Q_PROPERTY(double timeAccuracy READ getTimeAccuracy NOTIFY telemetryChanged)
    Q_PROPERTY(double ocxoFrequency READ getOcxoFrequency NOTIFY telemetryChanged)
    Q_PROPERTY(double ocxoStability READ getOcxoStability NOTIFY telemetryChanged)
    Q_PROPERTY(double temperature READ getTemperature NOTIFY telemetryChanged)
    Q_PROPERTY(QString syncSource READ getSyncSource NOTIFY telemetryChanged)
    
public:
    explicit TimingSyncSubsystem(const QString& id, const QString& name = "Timing & Sync",
                                 QObject* parent = nullptr);
    ~TimingSyncSubsystem() override = default;
    
    bool isGpsLocked() const;
    int getSatelliteCount() const;
    double getTimeAccuracy() const;     // Nanoseconds
    double getOcxoFrequency() const;    // MHz (deviation from nominal)
    double getOcxoStability() const;    // Parts per billion
    double getTemperature() const;      // Celsius
    QString getSyncSource() const;      // "GPS", "OCXO", "NTP", "MANUAL"
    double getPpsJitter() const;        // Nanoseconds
    bool isPpsValid() const;
    double getDop() const;              // Dilution of Precision
    
    // Timing fault codes
    static constexpr const char* FAULT_GPS_UNLOCK = "TIME-001";
    static constexpr const char* FAULT_LOW_SATELLITES = "TIME-002";
    static constexpr const char* FAULT_OCXO_DRIFT = "TIME-003";
    static constexpr const char* FAULT_PPS_INVALID = "TIME-004";
    static constexpr const char* FAULT_TIME_ACCURACY = "TIME-005";
    static constexpr const char* FAULT_ANTENNA_FAIL = "TIME-006";
    static constexpr const char* FAULT_OVERTEMP = "TIME-007";
    static constexpr const char* FAULT_HOLDOVER = "TIME-008";
    
protected:
    void initializeTelemetryParameters() override;
    HealthState computeHealthState() const override;
    double computeHealthScore() const override;
    QString computeStatusMessage() const override;
    void onDataUpdate(const QVariantMap& data) override;
    
private:
    static constexpr int SATELLITE_WARNING = 6;
    static constexpr int SATELLITE_CRITICAL = 4;
    static constexpr double ACCURACY_WARNING = 100.0;        // ns
    static constexpr double ACCURACY_CRITICAL = 1000.0;
    static constexpr double STABILITY_WARNING = 10.0;        // ppb
    static constexpr double STABILITY_CRITICAL = 100.0;
    static constexpr double TEMP_WARNING = 50.0;
    static constexpr double TEMP_CRITICAL = 60.0;
};

} // namespace RadarRMP

#endif // TIMINGSYNCSUBSYSTEM_H
