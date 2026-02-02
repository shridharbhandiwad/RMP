#include "subsystems/DataProcessorSubsystem.h"

namespace RadarRMP {

DataProcessorSubsystem::DataProcessorSubsystem(const QString& id, const QString& name, QObject* parent)
    : RadarSubsystem(id, name, SubsystemType::DataProcessor, parent)
{
    setDescription("Data processing and track management unit");
    initializeTelemetryParameters();
}

void DataProcessorSubsystem::initializeTelemetryParameters()
{
    TelemetryParameter cpuLoad("cpuLoad", "CPU Load", "%");
    cpuLoad.nominal = 45.0;
    cpuLoad.minValue = 0.0;
    cpuLoad.maxValue = 100.0;
    cpuLoad.warningHigh = 75.0;
    cpuLoad.criticalHigh = 90.0;
    cpuLoad.value = 45.0;
    addTelemetryParameter(cpuLoad);
    
    TelemetryParameter memoryUsage("memoryUsage", "Memory Usage", "%");
    memoryUsage.nominal = 35.0;
    memoryUsage.minValue = 0.0;
    memoryUsage.maxValue = 100.0;
    memoryUsage.warningHigh = 70.0;
    memoryUsage.criticalHigh = 85.0;
    memoryUsage.value = 35.0;
    addTelemetryParameter(memoryUsage);
    
    TelemetryParameter activeTracks("activeTracks", "Active Tracks", "");
    activeTracks.nominal = 50;
    activeTracks.minValue = 0;
    activeTracks.maxValue = 500;
    activeTracks.value = 50;
    addTelemetryParameter(activeTracks);
    
    TelemetryParameter maxTracks("maxTracks", "Max Tracks", "");
    maxTracks.value = 500;
    addTelemetryParameter(maxTracks);
    
    TelemetryParameter trackQuality("trackQuality", "Track Quality", "%");
    trackQuality.nominal = 95.0;
    trackQuality.minValue = 0.0;
    trackQuality.maxValue = 100.0;
    trackQuality.warningLow = 80.0;
    trackQuality.criticalLow = 60.0;
    trackQuality.value = 95.0;
    addTelemetryParameter(trackQuality);
    
    TelemetryParameter processingLatency("processingLatency", "Processing Latency", "ms");
    processingLatency.nominal = 50.0;
    processingLatency.minValue = 0.0;
    processingLatency.maxValue = 1000.0;
    processingLatency.warningHigh = 100.0;
    processingLatency.criticalHigh = 500.0;
    processingLatency.value = 50.0;
    addTelemetryParameter(processingLatency);
    
    TelemetryParameter updateRate("updateRate", "Update Rate", "Hz");
    updateRate.nominal = 10.0;
    updateRate.minValue = 0.0;
    updateRate.maxValue = 100.0;
    updateRate.value = 10.0;
    addTelemetryParameter(updateRate);
    
    TelemetryParameter droppedDetections("droppedDetections", "Dropped Detections", "");
    droppedDetections.value = 0;
    addTelemetryParameter(droppedDetections);
}

double DataProcessorSubsystem::getCpuLoad() const
{
    return getTelemetryValue("cpuLoad").toDouble();
}

double DataProcessorSubsystem::getMemoryUsage() const
{
    return getTelemetryValue("memoryUsage").toDouble();
}

int DataProcessorSubsystem::getActiveTracks() const
{
    return getTelemetryValue("activeTracks").toInt();
}

int DataProcessorSubsystem::getMaxTracks() const
{
    return getTelemetryValue("maxTracks").toInt();
}

double DataProcessorSubsystem::getTrackQuality() const
{
    return getTelemetryValue("trackQuality").toDouble();
}

double DataProcessorSubsystem::getProcessingLatency() const
{
    return getTelemetryValue("processingLatency").toDouble();
}

double DataProcessorSubsystem::getUpdateRate() const
{
    return getTelemetryValue("updateRate").toDouble();
}

int DataProcessorSubsystem::getDroppedDetections() const
{
    return getTelemetryValue("droppedDetections").toInt();
}

HealthState DataProcessorSubsystem::computeHealthState() const
{
    if (!isEnabled()) {
        return HealthState::UNKNOWN;
    }
    
    double cpu = getCpuLoad();
    double mem = getMemoryUsage();
    double lat = getProcessingLatency();
    double trackCapacity = (double)getActiveTracks() / getMaxTracks() * 100.0;
    
    if (cpu >= CPU_CRITICAL || mem >= MEMORY_CRITICAL || 
        lat >= LATENCY_CRITICAL || trackCapacity >= TRACK_CAPACITY_CRITICAL) {
        return HealthState::FAIL;
    }
    
    if (cpu >= CPU_WARNING || mem >= MEMORY_WARNING || 
        lat >= LATENCY_WARNING || trackCapacity >= TRACK_CAPACITY_WARNING) {
        return HealthState::DEGRADED;
    }
    
    if (hasFaults()) {
        return HealthState::DEGRADED;
    }
    
    return HealthState::OK;
}

double DataProcessorSubsystem::computeHealthScore() const
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
    
    double lat = getProcessingLatency();
    if (lat >= LATENCY_CRITICAL) {
        score -= 25;
    } else if (lat >= LATENCY_WARNING) {
        score -= 12 * (lat - LATENCY_WARNING) / (LATENCY_CRITICAL - LATENCY_WARNING);
    }
    
    double trackCapacity = (double)getActiveTracks() / getMaxTracks() * 100.0;
    if (trackCapacity >= TRACK_CAPACITY_CRITICAL) {
        score -= 20;
    } else if (trackCapacity >= TRACK_CAPACITY_WARNING) {
        score -= 10 * (trackCapacity - TRACK_CAPACITY_WARNING) / (TRACK_CAPACITY_CRITICAL - TRACK_CAPACITY_WARNING);
    }
    
    score -= getFaultCount() * 5;
    
    return qMax(0.0, qMin(100.0, score));
}

QString DataProcessorSubsystem::computeStatusMessage() const
{
    if (!isEnabled()) {
        return "Data Processor disabled";
    }
    
    if (getCpuLoad() >= CPU_CRITICAL) {
        return "CRITICAL: CPU overload";
    }
    
    if (getMemoryUsage() >= MEMORY_CRITICAL) {
        return "CRITICAL: Memory exhausted";
    }
    
    double trackCapacity = (double)getActiveTracks() / getMaxTracks() * 100.0;
    if (trackCapacity >= TRACK_CAPACITY_CRITICAL) {
        return "CRITICAL: Track capacity exceeded";
    }
    
    if (trackCapacity >= TRACK_CAPACITY_WARNING) {
        return "WARNING: High track load";
    }
    
    return QString("Tracking %1/%2 targets, Quality: %3%")
           .arg(getActiveTracks())
           .arg(getMaxTracks())
           .arg(getTrackQuality(), 0, 'f', 0);
}

void DataProcessorSubsystem::onDataUpdate(const QVariantMap& data)
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
    
    if (data.contains("activeTracks") && data.contains("maxTracks")) {
        int active = data["activeTracks"].toInt();
        int max = data["maxTracks"].toInt();
        if (max > 0 && (double)active / max * 100.0 >= TRACK_CAPACITY_CRITICAL) {
            addFault(FaultCode(FAULT_TRACK_OVERFLOW, "Track capacity exceeded", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_TRACK_OVERFLOW);
        }
    }
}

} // namespace RadarRMP
