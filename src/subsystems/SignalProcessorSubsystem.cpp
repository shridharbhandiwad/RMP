#include "subsystems/SignalProcessorSubsystem.h"

namespace RadarRMP {

SignalProcessorSubsystem::SignalProcessorSubsystem(const QString& id, const QString& name, QObject* parent)
    : RadarSubsystem(id, name, SubsystemType::SignalProcessor, parent)
{
    setDescription("Digital signal processing unit with FPGA-based pulse compression and Doppler processing");
    initializeTelemetryParameters();
}

void SignalProcessorSubsystem::initializeTelemetryParameters()
{
    TelemetryParameter cpuLoad("cpuLoad", "CPU Load", "%");
    cpuLoad.nominal = 50.0;
    cpuLoad.minValue = 0.0;
    cpuLoad.maxValue = 100.0;
    cpuLoad.warningHigh = 80.0;
    cpuLoad.criticalHigh = 95.0;
    cpuLoad.value = 50.0;
    addTelemetryParameter(cpuLoad);
    
    TelemetryParameter memoryUsage("memoryUsage", "Memory Usage", "%");
    memoryUsage.nominal = 40.0;
    memoryUsage.minValue = 0.0;
    memoryUsage.maxValue = 100.0;
    memoryUsage.warningHigh = 75.0;
    memoryUsage.criticalHigh = 90.0;
    memoryUsage.value = 40.0;
    addTelemetryParameter(memoryUsage);
    
    TelemetryParameter throughput("throughput", "Throughput", "MSPS");
    throughput.nominal = 100.0;
    throughput.minValue = 0.0;
    throughput.maxValue = 200.0;
    throughput.warningLow = 80.0;
    throughput.criticalLow = 50.0;
    throughput.value = 100.0;
    addTelemetryParameter(throughput);
    
    TelemetryParameter temperature("temperature", "Temperature", "°C");
    temperature.nominal = 55.0;
    temperature.minValue = 0.0;
    temperature.maxValue = 100.0;
    temperature.warningHigh = 70.0;
    temperature.criticalHigh = 85.0;
    temperature.value = 55.0;
    addTelemetryParameter(temperature);
    
    TelemetryParameter latency("latency", "Latency", "ms");
    latency.nominal = 5.0;
    latency.minValue = 0.0;
    latency.maxValue = 100.0;
    latency.warningHigh = 10.0;
    latency.criticalHigh = 50.0;
    latency.value = 5.0;
    addTelemetryParameter(latency);
    
    TelemetryParameter droppedPackets("droppedPackets", "Dropped Packets", "");
    droppedPackets.nominal = 0;
    droppedPackets.minValue = 0;
    droppedPackets.maxValue = 10000;
    droppedPackets.value = 0;
    addTelemetryParameter(droppedPackets);
    
    TelemetryParameter fpgaHealthy("fpgaHealthy", "FPGA Healthy", "");
    fpgaHealthy.value = true;
    addTelemetryParameter(fpgaHealthy);
    
    TelemetryParameter dspUtilization("dspUtilization", "DSP Utilization", "%");
    dspUtilization.nominal = 60.0;
    dspUtilization.minValue = 0.0;
    dspUtilization.maxValue = 100.0;
    dspUtilization.warningHigh = 85.0;
    dspUtilization.criticalHigh = 95.0;
    dspUtilization.value = 60.0;
    addTelemetryParameter(dspUtilization);
}

double SignalProcessorSubsystem::getCpuLoad() const
{
    return getTelemetryValue("cpuLoad").toDouble();
}

double SignalProcessorSubsystem::getMemoryUsage() const
{
    return getTelemetryValue("memoryUsage").toDouble();
}

double SignalProcessorSubsystem::getThroughput() const
{
    return getTelemetryValue("throughput").toDouble();
}

double SignalProcessorSubsystem::getTemperature() const
{
    return getTelemetryValue("temperature").toDouble();
}

double SignalProcessorSubsystem::getLatency() const
{
    return getTelemetryValue("latency").toDouble();
}

int SignalProcessorSubsystem::getDroppedPackets() const
{
    return getTelemetryValue("droppedPackets").toInt();
}

bool SignalProcessorSubsystem::isFpgaHealthy() const
{
    return getTelemetryValue("fpgaHealthy").toBool();
}

double SignalProcessorSubsystem::getDspUtilization() const
{
    return getTelemetryValue("dspUtilization").toDouble();
}

HealthState SignalProcessorSubsystem::computeHealthState() const
{
    if (!isEnabled()) {
        return HealthState::UNKNOWN;
    }
    
    double cpu = getCpuLoad();
    double mem = getMemoryUsage();
    double temp = getTemperature();
    double lat = getLatency();
    
    if (cpu >= CPU_CRITICAL || mem >= MEMORY_CRITICAL || 
        temp >= TEMP_CRITICAL || lat >= LATENCY_CRITICAL || !isFpgaHealthy()) {
        return HealthState::FAIL;
    }
    
    if (cpu >= CPU_WARNING || mem >= MEMORY_WARNING || 
        temp >= TEMP_WARNING || lat >= LATENCY_WARNING) {
        return HealthState::DEGRADED;
    }
    
    if (hasFaults()) {
        return HealthState::DEGRADED;
    }
    
    return HealthState::OK;
}

double SignalProcessorSubsystem::computeHealthScore() const
{
    double score = 100.0;
    
    double cpu = getCpuLoad();
    if (cpu >= CPU_CRITICAL) {
        score -= 25;
    } else if (cpu >= CPU_WARNING) {
        score -= 12 * (cpu - CPU_WARNING) / (CPU_CRITICAL - CPU_WARNING);
    }
    
    double mem = getMemoryUsage();
    if (mem >= MEMORY_CRITICAL) {
        score -= 25;
    } else if (mem >= MEMORY_WARNING) {
        score -= 12 * (mem - MEMORY_WARNING) / (MEMORY_CRITICAL - MEMORY_WARNING);
    }
    
    double temp = getTemperature();
    if (temp >= TEMP_CRITICAL) {
        score -= 25;
    } else if (temp >= TEMP_WARNING) {
        score -= 12 * (temp - TEMP_WARNING) / (TEMP_CRITICAL - TEMP_WARNING);
    }
    
    double lat = getLatency();
    if (lat >= LATENCY_CRITICAL) {
        score -= 20;
    } else if (lat >= LATENCY_WARNING) {
        score -= 10 * (lat - LATENCY_WARNING) / (LATENCY_CRITICAL - LATENCY_WARNING);
    }
    
    if (!isFpgaHealthy()) {
        score -= 30;
    }
    
    score -= getFaultCount() * 5;
    
    return qMax(0.0, qMin(100.0, score));
}

QString SignalProcessorSubsystem::computeStatusMessage() const
{
    if (!isEnabled()) {
        return "Signal Processor disabled";
    }
    
    if (!isFpgaHealthy()) {
        return "CRITICAL: FPGA error";
    }
    
    if (getCpuLoad() >= CPU_CRITICAL) {
        return "CRITICAL: CPU overload";
    }
    
    if (getMemoryUsage() >= MEMORY_CRITICAL) {
        return "CRITICAL: Memory exhausted";
    }
    
    if (getCpuLoad() >= CPU_WARNING) {
        return "WARNING: High CPU load";
    }
    
    if (getMemoryUsage() >= MEMORY_WARNING) {
        return "WARNING: High memory usage";
    }
    
    return QString("Processing - %1 MSPS, Lat: %2ms")
           .arg(getThroughput(), 0, 'f', 0)
           .arg(getLatency(), 0, 'f', 1);
}

void SignalProcessorSubsystem::onDataUpdate(const QVariantMap& data)
{
    if (data.contains("cpuLoad")) {
        double cpu = data["cpuLoad"].toDouble();
        if (cpu >= CPU_CRITICAL) {
            addFault(FaultCode(FAULT_CPU_OVERLOAD, "CPU overload", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_CPU_OVERLOAD);
        }
    }
    
    if (data.contains("memoryUsage")) {
        double mem = data["memoryUsage"].toDouble();
        if (mem >= MEMORY_CRITICAL) {
            addFault(FaultCode(FAULT_MEMORY_FULL, "Memory exhausted", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_MEMORY_FULL);
        }
    }
    
    if (data.contains("fpgaHealthy")) {
        bool healthy = data["fpgaHealthy"].toBool();
        if (!healthy) {
            addFault(FaultCode(FAULT_FPGA_ERROR, "FPGA error", 
                              FaultSeverity::FATAL, getId()));
        } else {
            clearFault(FAULT_FPGA_ERROR);
        }
    }
}

} // namespace RadarRMP
