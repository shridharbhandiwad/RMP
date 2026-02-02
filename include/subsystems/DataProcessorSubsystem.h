#ifndef DATAPROCESSORSUBSYSTEM_H
#define DATAPROCESSORSUBSYSTEM_H

#include "core/RadarSubsystem.h"

namespace RadarRMP {

/**
 * @brief Data Processor / Tracker subsystem implementation
 * 
 * Monitors and reports health of the data processing and tracking unit:
 * - Track processing load
 * - Number of active tracks
 * - Track quality metrics
 * - Processing latency
 * - System resources
 */
class DataProcessorSubsystem : public RadarSubsystem {
    Q_OBJECT
    
    Q_PROPERTY(double cpuLoad READ getCpuLoad NOTIFY telemetryChanged)
    Q_PROPERTY(double memoryUsage READ getMemoryUsage NOTIFY telemetryChanged)
    Q_PROPERTY(int activeTracks READ getActiveTracks NOTIFY telemetryChanged)
    Q_PROPERTY(int maxTracks READ getMaxTracks NOTIFY telemetryChanged)
    Q_PROPERTY(double trackQuality READ getTrackQuality NOTIFY telemetryChanged)
    Q_PROPERTY(double processingLatency READ getProcessingLatency NOTIFY telemetryChanged)
    
public:
    explicit DataProcessorSubsystem(const QString& id, const QString& name = "Data Processor",
                                    QObject* parent = nullptr);
    ~DataProcessorSubsystem() override = default;
    
    double getCpuLoad() const;            // Percentage
    double getMemoryUsage() const;        // Percentage
    int getActiveTracks() const;          // Count
    int getMaxTracks() const;             // Max capacity
    double getTrackQuality() const;       // 0-100%
    double getProcessingLatency() const;  // ms
    double getUpdateRate() const;         // Hz
    int getDroppedDetections() const;
    
    // Data Processor fault codes
    static constexpr const char* FAULT_CPU_OVERLOAD = "DP-001";
    static constexpr const char* FAULT_MEMORY_FULL = "DP-002";
    static constexpr const char* FAULT_TRACK_OVERFLOW = "DP-003";
    static constexpr const char* FAULT_QUALITY_LOW = "DP-004";
    static constexpr const char* FAULT_LATENCY_HIGH = "DP-005";
    static constexpr const char* FAULT_DATA_LOSS = "DP-006";
    static constexpr const char* FAULT_ALGORITHM_ERROR = "DP-007";
    
protected:
    void initializeTelemetryParameters() override;
    HealthState computeHealthState() const override;
    double computeHealthScore() const override;
    QString computeStatusMessage() const override;
    void onDataUpdate(const QVariantMap& data) override;
    
private:
    static constexpr double CPU_WARNING = 75.0;
    static constexpr double CPU_CRITICAL = 90.0;
    static constexpr double MEMORY_WARNING = 70.0;
    static constexpr double MEMORY_CRITICAL = 85.0;
    static constexpr double TRACK_CAPACITY_WARNING = 80.0;  // % of max
    static constexpr double TRACK_CAPACITY_CRITICAL = 95.0;
    static constexpr double LATENCY_WARNING = 100.0;        // ms
    static constexpr double LATENCY_CRITICAL = 500.0;
};

} // namespace RadarRMP

#endif // DATAPROCESSORSUBSYSTEM_H
