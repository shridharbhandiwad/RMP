#ifndef SIGNALPROCESSORSUBSYSTEM_H
#define SIGNALPROCESSORSUBSYSTEM_H

#include "core/RadarSubsystem.h"

namespace RadarRMP {

/**
 * @brief Signal Processor subsystem implementation
 * 
 * Monitors and reports health of the signal processing unit:
 * - Processing load
 * - FPGA status
 * - DSP health
 * - Memory utilization
 * - Throughput metrics
 */
class SignalProcessorSubsystem : public RadarSubsystem {
    Q_OBJECT
    
    Q_PROPERTY(double cpuLoad READ getCpuLoad NOTIFY telemetryChanged)
    Q_PROPERTY(double memoryUsage READ getMemoryUsage NOTIFY telemetryChanged)
    Q_PROPERTY(double throughput READ getThroughput NOTIFY telemetryChanged)
    Q_PROPERTY(double temperature READ getTemperature NOTIFY telemetryChanged)
    Q_PROPERTY(double latency READ getLatency NOTIFY telemetryChanged)
    Q_PROPERTY(int droppedPackets READ getDroppedPackets NOTIFY telemetryChanged)
    
public:
    explicit SignalProcessorSubsystem(const QString& id, const QString& name = "Signal Processor",
                                      QObject* parent = nullptr);
    ~SignalProcessorSubsystem() override = default;
    
    double getCpuLoad() const;        // Percentage
    double getMemoryUsage() const;    // Percentage
    double getThroughput() const;     // MSPS
    double getTemperature() const;    // Celsius
    double getLatency() const;        // Milliseconds
    int getDroppedPackets() const;
    bool isFpgaHealthy() const;
    double getDspUtilization() const; // Percentage
    
    // Signal Processor fault codes
    static constexpr const char* FAULT_CPU_OVERLOAD = "SP-001";
    static constexpr const char* FAULT_MEMORY_FULL = "SP-002";
    static constexpr const char* FAULT_FPGA_ERROR = "SP-003";
    static constexpr const char* FAULT_DSP_ERROR = "SP-004";
    static constexpr const char* FAULT_THROUGHPUT_LOW = "SP-005";
    static constexpr const char* FAULT_LATENCY_HIGH = "SP-006";
    static constexpr const char* FAULT_OVERTEMP = "SP-007";
    static constexpr const char* FAULT_DATA_LOSS = "SP-008";
    
protected:
    void initializeTelemetryParameters() override;
    HealthState computeHealthState() const override;
    double computeHealthScore() const override;
    QString computeStatusMessage() const override;
    void onDataUpdate(const QVariantMap& data) override;
    
private:
    static constexpr double CPU_WARNING = 80.0;          // Percentage
    static constexpr double CPU_CRITICAL = 95.0;
    static constexpr double MEMORY_WARNING = 75.0;
    static constexpr double MEMORY_CRITICAL = 90.0;
    static constexpr double LATENCY_WARNING = 10.0;      // ms
    static constexpr double LATENCY_CRITICAL = 50.0;
    static constexpr double TEMP_WARNING = 70.0;
    static constexpr double TEMP_CRITICAL = 85.0;
};

} // namespace RadarRMP

#endif // SIGNALPROCESSORSUBSYSTEM_H
