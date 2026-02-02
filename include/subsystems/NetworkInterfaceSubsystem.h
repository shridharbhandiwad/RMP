#ifndef NETWORKINTERFACESUBSYSTEM_H
#define NETWORKINTERFACESUBSYSTEM_H

#include "core/RadarSubsystem.h"

namespace RadarRMP {

/**
 * @brief Network & Communication Interface subsystem implementation
 * 
 * Monitors and reports health of network interfaces:
 * - Link status
 * - Bandwidth utilization
 * - Packet loss / errors
 * - Latency
 * - Connection status to C2 systems
 */
class NetworkInterfaceSubsystem : public RadarSubsystem {
    Q_OBJECT
    
    Q_PROPERTY(bool linkUp READ isLinkUp NOTIFY telemetryChanged)
    Q_PROPERTY(double bandwidth READ getBandwidth NOTIFY telemetryChanged)
    Q_PROPERTY(double utilization READ getUtilization NOTIFY telemetryChanged)
    Q_PROPERTY(double packetLoss READ getPacketLoss NOTIFY telemetryChanged)
    Q_PROPERTY(double latency READ getLatency NOTIFY telemetryChanged)
    Q_PROPERTY(int errorCount READ getErrorCount NOTIFY telemetryChanged)
    Q_PROPERTY(QString connectionStatus READ getConnectionStatus NOTIFY telemetryChanged)
    
public:
    explicit NetworkInterfaceSubsystem(const QString& id, const QString& name = "Network Interface",
                                       QObject* parent = nullptr);
    ~NetworkInterfaceSubsystem() override = default;
    
    bool isLinkUp() const;
    double getBandwidth() const;        // Mbps
    double getUtilization() const;      // Percentage
    double getPacketLoss() const;       // Percentage
    double getLatency() const;          // Milliseconds
    int getErrorCount() const;
    QString getConnectionStatus() const; // "CONNECTED", "DISCONNECTED", "DEGRADED"
    double getTxRate() const;           // Mbps
    double getRxRate() const;           // Mbps
    int getActiveConnections() const;
    
    // Network fault codes
    static constexpr const char* FAULT_LINK_DOWN = "NET-001";
    static constexpr const char* FAULT_HIGH_PACKET_LOSS = "NET-002";
    static constexpr const char* FAULT_HIGH_LATENCY = "NET-003";
    static constexpr const char* FAULT_BANDWIDTH_EXCEEDED = "NET-004";
    static constexpr const char* FAULT_C2_DISCONNECT = "NET-005";
    static constexpr const char* FAULT_BUFFER_OVERFLOW = "NET-006";
    static constexpr const char* FAULT_CRC_ERRORS = "NET-007";
    static constexpr const char* FAULT_INTERFACE_ERROR = "NET-008";
    
protected:
    void initializeTelemetryParameters() override;
    HealthState computeHealthState() const override;
    double computeHealthScore() const override;
    QString computeStatusMessage() const override;
    void onDataUpdate(const QVariantMap& data) override;
    
private:
    static constexpr double PACKET_LOSS_WARNING = 0.1;       // Percentage
    static constexpr double PACKET_LOSS_CRITICAL = 1.0;
    static constexpr double LATENCY_WARNING = 50.0;          // ms
    static constexpr double LATENCY_CRITICAL = 200.0;
    static constexpr double UTILIZATION_WARNING = 80.0;      // Percentage
    static constexpr double UTILIZATION_CRITICAL = 95.0;
};

} // namespace RadarRMP

#endif // NETWORKINTERFACESUBSYSTEM_H
