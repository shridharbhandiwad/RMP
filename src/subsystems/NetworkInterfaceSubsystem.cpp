#include "subsystems/NetworkInterfaceSubsystem.h"

namespace RadarRMP {

NetworkInterfaceSubsystem::NetworkInterfaceSubsystem(const QString& id, const QString& name, QObject* parent)
    : RadarSubsystem(id, name, SubsystemType::NetworkInterface, parent)
{
    setDescription("Network communication interface for C2 connectivity");
    initializeTelemetryParameters();
}

void NetworkInterfaceSubsystem::initializeTelemetryParameters()
{
    TelemetryParameter linkUp("linkUp", "Link Up", "");
    linkUp.value = true;
    addTelemetryParameter(linkUp);
    
    TelemetryParameter bandwidth("bandwidth", "Bandwidth", "Mbps");
    bandwidth.nominal = 1000.0;
    bandwidth.minValue = 0.0;
    bandwidth.maxValue = 10000.0;
    bandwidth.value = 1000.0;
    addTelemetryParameter(bandwidth);
    
    TelemetryParameter utilization("utilization", "Utilization", "%");
    utilization.nominal = 30.0;
    utilization.minValue = 0.0;
    utilization.maxValue = 100.0;
    utilization.warningHigh = 80.0;
    utilization.criticalHigh = 95.0;
    utilization.value = 30.0;
    addTelemetryParameter(utilization);
    
    TelemetryParameter packetLoss("packetLoss", "Packet Loss", "%");
    packetLoss.nominal = 0.0;
    packetLoss.minValue = 0.0;
    packetLoss.maxValue = 100.0;
    packetLoss.warningHigh = 0.1;
    packetLoss.criticalHigh = 1.0;
    packetLoss.value = 0.0;
    addTelemetryParameter(packetLoss);
    
    TelemetryParameter latency("latency", "Latency", "ms");
    latency.nominal = 5.0;
    latency.minValue = 0.0;
    latency.maxValue = 1000.0;
    latency.warningHigh = 50.0;
    latency.criticalHigh = 200.0;
    latency.value = 5.0;
    addTelemetryParameter(latency);
    
    TelemetryParameter errorCount("errorCount", "Error Count", "");
    errorCount.nominal = 0;
    errorCount.minValue = 0;
    errorCount.maxValue = 100000;
    errorCount.value = 0;
    addTelemetryParameter(errorCount);
    
    TelemetryParameter connectionStatus("connectionStatus", "Connection Status", "");
    connectionStatus.value = "CONNECTED";
    addTelemetryParameter(connectionStatus);
    
    TelemetryParameter txRate("txRate", "TX Rate", "Mbps");
    txRate.nominal = 100.0;
    txRate.minValue = 0.0;
    txRate.maxValue = 10000.0;
    txRate.value = 100.0;
    addTelemetryParameter(txRate);
    
    TelemetryParameter rxRate("rxRate", "RX Rate", "Mbps");
    rxRate.nominal = 150.0;
    rxRate.minValue = 0.0;
    rxRate.maxValue = 10000.0;
    rxRate.value = 150.0;
    addTelemetryParameter(rxRate);
    
    TelemetryParameter activeConnections("activeConnections", "Active Connections", "");
    activeConnections.nominal = 5;
    activeConnections.minValue = 0;
    activeConnections.maxValue = 100;
    activeConnections.value = 5;
    addTelemetryParameter(activeConnections);
}

bool NetworkInterfaceSubsystem::isLinkUp() const
{
    return getTelemetryValue("linkUp").toBool();
}

double NetworkInterfaceSubsystem::getBandwidth() const
{
    return getTelemetryValue("bandwidth").toDouble();
}

double NetworkInterfaceSubsystem::getUtilization() const
{
    return getTelemetryValue("utilization").toDouble();
}

double NetworkInterfaceSubsystem::getPacketLoss() const
{
    return getTelemetryValue("packetLoss").toDouble();
}

double NetworkInterfaceSubsystem::getLatency() const
{
    return getTelemetryValue("latency").toDouble();
}

int NetworkInterfaceSubsystem::getErrorCount() const
{
    return getTelemetryValue("errorCount").toInt();
}

QString NetworkInterfaceSubsystem::getConnectionStatus() const
{
    return getTelemetryValue("connectionStatus").toString();
}

double NetworkInterfaceSubsystem::getTxRate() const
{
    return getTelemetryValue("txRate").toDouble();
}

double NetworkInterfaceSubsystem::getRxRate() const
{
    return getTelemetryValue("rxRate").toDouble();
}

int NetworkInterfaceSubsystem::getActiveConnections() const
{
    return getTelemetryValue("activeConnections").toInt();
}

HealthState NetworkInterfaceSubsystem::computeHealthState() const
{
    if (!isEnabled()) {
        return HealthState::UNKNOWN;
    }
    
    if (!isLinkUp()) {
        return HealthState::FAIL;
    }
    
    double loss = getPacketLoss();
    double lat = getLatency();
    double util = getUtilization();
    
    if (loss >= PACKET_LOSS_CRITICAL || lat >= LATENCY_CRITICAL ||
        util >= UTILIZATION_CRITICAL) {
        return HealthState::FAIL;
    }
    
    if (loss >= PACKET_LOSS_WARNING || lat >= LATENCY_WARNING ||
        util >= UTILIZATION_WARNING) {
        return HealthState::DEGRADED;
    }
    
    if (getConnectionStatus() == "DEGRADED") {
        return HealthState::DEGRADED;
    }
    
    if (hasFaults()) {
        return HealthState::DEGRADED;
    }
    
    return HealthState::OK;
}

double NetworkInterfaceSubsystem::computeHealthScore() const
{
    double score = 100.0;
    
    if (!isLinkUp()) {
        return 0.0;
    }
    
    double loss = getPacketLoss();
    if (loss >= PACKET_LOSS_CRITICAL) {
        score -= 35;
    } else if (loss >= PACKET_LOSS_WARNING) {
        score -= 15 * (loss - PACKET_LOSS_WARNING) / (PACKET_LOSS_CRITICAL - PACKET_LOSS_WARNING);
    }
    
    double lat = getLatency();
    if (lat >= LATENCY_CRITICAL) {
        score -= 30;
    } else if (lat >= LATENCY_WARNING) {
        score -= 15 * (lat - LATENCY_WARNING) / (LATENCY_CRITICAL - LATENCY_WARNING);
    }
    
    double util = getUtilization();
    if (util >= UTILIZATION_CRITICAL) {
        score -= 25;
    } else if (util >= UTILIZATION_WARNING) {
        score -= 12 * (util - UTILIZATION_WARNING) / (UTILIZATION_CRITICAL - UTILIZATION_WARNING);
    }
    
    score -= getFaultCount() * 5;
    
    return qMax(0.0, qMin(100.0, score));
}

QString NetworkInterfaceSubsystem::computeStatusMessage() const
{
    if (!isEnabled()) {
        return "Network Interface disabled";
    }
    
    if (!isLinkUp()) {
        return "CRITICAL: Link down";
    }
    
    double loss = getPacketLoss();
    if (loss >= PACKET_LOSS_CRITICAL) {
        return QString("CRITICAL: High packet loss (%1%)").arg(loss, 0, 'f', 2);
    }
    
    double lat = getLatency();
    if (lat >= LATENCY_CRITICAL) {
        return QString("CRITICAL: High latency (%1 ms)").arg(lat, 0, 'f', 0);
    }
    
    if (loss >= PACKET_LOSS_WARNING) {
        return QString("WARNING: Packet loss (%1%)").arg(loss, 0, 'f', 2);
    }
    
    if (lat >= LATENCY_WARNING) {
        return QString("WARNING: Elevated latency (%1 ms)").arg(lat, 0, 'f', 0);
    }
    
    return QString("Connected - %1 Mbps, Latency: %2 ms")
           .arg(getBandwidth(), 0, 'f', 0)
           .arg(lat, 0, 'f', 1);
}

void NetworkInterfaceSubsystem::onDataUpdate(const QVariantMap& data)
{
    if (data.contains("linkUp")) {
        bool up = data["linkUp"].toBool();
        if (!up) {
            addFault(FaultCode(FAULT_LINK_DOWN, "Network link down", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_LINK_DOWN);
        }
    }
    
    if (data.contains("packetLoss")) {
        double loss = data["packetLoss"].toDouble();
        if (loss >= PACKET_LOSS_CRITICAL) {
            addFault(FaultCode(FAULT_HIGH_PACKET_LOSS, "High packet loss", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_HIGH_PACKET_LOSS);
        }
    }
    
    if (data.contains("latency")) {
        double lat = data["latency"].toDouble();
        if (lat >= LATENCY_CRITICAL) {
            addFault(FaultCode(FAULT_HIGH_LATENCY, "High network latency", 
                              FaultSeverity::WARNING, getId()));
        } else {
            clearFault(FAULT_HIGH_LATENCY);
        }
    }
    
    if (data.contains("connectionStatus")) {
        QString status = data["connectionStatus"].toString();
        if (status == "DISCONNECTED") {
            addFault(FaultCode(FAULT_C2_DISCONNECT, "C2 system disconnected", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_C2_DISCONNECT);
        }
    }
}

} // namespace RadarRMP
