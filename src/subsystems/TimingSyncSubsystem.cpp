#include "subsystems/TimingSyncSubsystem.h"

namespace RadarRMP {

TimingSyncSubsystem::TimingSyncSubsystem(const QString& id, const QString& name, QObject* parent)
    : RadarSubsystem(id, name, SubsystemType::TimingSync, parent)
{
    setDescription("Timing and synchronization unit with GPS and OCXO");
    initializeTelemetryParameters();
}

void TimingSyncSubsystem::initializeTelemetryParameters()
{
    TelemetryParameter gpsLocked("gpsLocked", "GPS Locked", "");
    gpsLocked.value = true;
    addTelemetryParameter(gpsLocked);
    
    TelemetryParameter satelliteCount("satelliteCount", "Satellites", "");
    satelliteCount.nominal = 12;
    satelliteCount.minValue = 0;
    satelliteCount.maxValue = 24;
    satelliteCount.warningLow = 6;
    satelliteCount.criticalLow = 4;
    satelliteCount.value = 12;
    addTelemetryParameter(satelliteCount);
    
    TelemetryParameter timeAccuracy("timeAccuracy", "Time Accuracy", "ns");
    timeAccuracy.nominal = 20.0;
    timeAccuracy.minValue = 0.0;
    timeAccuracy.maxValue = 10000.0;
    timeAccuracy.warningHigh = 100.0;
    timeAccuracy.criticalHigh = 1000.0;
    timeAccuracy.value = 20.0;
    addTelemetryParameter(timeAccuracy);
    
    TelemetryParameter ocxoFrequency("ocxoFrequency", "OCXO Frequency", "MHz");
    ocxoFrequency.nominal = 10.0;
    ocxoFrequency.minValue = 9.99999;
    ocxoFrequency.maxValue = 10.00001;
    ocxoFrequency.value = 10.0;
    addTelemetryParameter(ocxoFrequency);
    
    TelemetryParameter ocxoStability("ocxoStability", "OCXO Stability", "ppb");
    ocxoStability.nominal = 1.0;
    ocxoStability.minValue = 0.0;
    ocxoStability.maxValue = 1000.0;
    ocxoStability.warningHigh = 10.0;
    ocxoStability.criticalHigh = 100.0;
    ocxoStability.value = 1.0;
    addTelemetryParameter(ocxoStability);
    
    TelemetryParameter temperature("temperature", "Temperature", "°C");
    temperature.nominal = 40.0;
    temperature.minValue = 0.0;
    temperature.maxValue = 80.0;
    temperature.warningHigh = 50.0;
    temperature.criticalHigh = 60.0;
    temperature.value = 40.0;
    addTelemetryParameter(temperature);
    
    TelemetryParameter syncSource("syncSource", "Sync Source", "");
    syncSource.value = "GPS";
    addTelemetryParameter(syncSource);
    
    TelemetryParameter ppsJitter("ppsJitter", "PPS Jitter", "ns");
    ppsJitter.nominal = 5.0;
    ppsJitter.minValue = 0.0;
    ppsJitter.maxValue = 1000.0;
    ppsJitter.value = 5.0;
    addTelemetryParameter(ppsJitter);
    
    TelemetryParameter ppsValid("ppsValid", "PPS Valid", "");
    ppsValid.value = true;
    addTelemetryParameter(ppsValid);
    
    TelemetryParameter dop("dop", "DOP", "");
    dop.nominal = 1.0;
    dop.minValue = 0.0;
    dop.maxValue = 20.0;
    dop.warningHigh = 5.0;
    dop.criticalHigh = 10.0;
    dop.value = 1.0;
    addTelemetryParameter(dop);
}

bool TimingSyncSubsystem::isGpsLocked() const
{
    return getTelemetryValue("gpsLocked").toBool();
}

int TimingSyncSubsystem::getSatelliteCount() const
{
    return getTelemetryValue("satelliteCount").toInt();
}

double TimingSyncSubsystem::getTimeAccuracy() const
{
    return getTelemetryValue("timeAccuracy").toDouble();
}

double TimingSyncSubsystem::getOcxoFrequency() const
{
    return getTelemetryValue("ocxoFrequency").toDouble();
}

double TimingSyncSubsystem::getOcxoStability() const
{
    return getTelemetryValue("ocxoStability").toDouble();
}

double TimingSyncSubsystem::getTemperature() const
{
    return getTelemetryValue("temperature").toDouble();
}

QString TimingSyncSubsystem::getSyncSource() const
{
    return getTelemetryValue("syncSource").toString();
}

double TimingSyncSubsystem::getPpsJitter() const
{
    return getTelemetryValue("ppsJitter").toDouble();
}

bool TimingSyncSubsystem::isPpsValid() const
{
    return getTelemetryValue("ppsValid").toBool();
}

double TimingSyncSubsystem::getDop() const
{
    return getTelemetryValue("dop").toDouble();
}

HealthState TimingSyncSubsystem::computeHealthState() const
{
    if (!isEnabled()) {
        return HealthState::UNKNOWN;
    }
    
    int sats = getSatelliteCount();
    double accuracy = getTimeAccuracy();
    double stability = getOcxoStability();
    double temp = getTemperature();
    
    if (!isGpsLocked() || sats < SATELLITE_CRITICAL || 
        accuracy >= ACCURACY_CRITICAL || stability >= STABILITY_CRITICAL ||
        temp >= TEMP_CRITICAL || !isPpsValid()) {
        return HealthState::FAIL;
    }
    
    if (sats < SATELLITE_WARNING || accuracy >= ACCURACY_WARNING ||
        stability >= STABILITY_WARNING || temp >= TEMP_WARNING) {
        return HealthState::DEGRADED;
    }
    
    if (hasFaults()) {
        return HealthState::DEGRADED;
    }
    
    return HealthState::OK;
}

double TimingSyncSubsystem::computeHealthScore() const
{
    double score = 100.0;
    
    if (!isGpsLocked()) {
        score -= 30;
    }
    
    int sats = getSatelliteCount();
    if (sats < SATELLITE_CRITICAL) {
        score -= 25;
    } else if (sats < SATELLITE_WARNING) {
        score -= 12 * (SATELLITE_WARNING - sats) / (SATELLITE_WARNING - SATELLITE_CRITICAL);
    }
    
    double accuracy = getTimeAccuracy();
    if (accuracy >= ACCURACY_CRITICAL) {
        score -= 25;
    } else if (accuracy >= ACCURACY_WARNING) {
        score -= 12 * (accuracy - ACCURACY_WARNING) / (ACCURACY_CRITICAL - ACCURACY_WARNING);
    }
    
    double stability = getOcxoStability();
    if (stability >= STABILITY_CRITICAL) {
        score -= 20;
    } else if (stability >= STABILITY_WARNING) {
        score -= 10 * (stability - STABILITY_WARNING) / (STABILITY_CRITICAL - STABILITY_WARNING);
    }
    
    if (!isPpsValid()) {
        score -= 20;
    }
    
    score -= getFaultCount() * 5;
    
    return qMax(0.0, qMin(100.0, score));
}

QString TimingSyncSubsystem::computeStatusMessage() const
{
    if (!isEnabled()) {
        return "Timing System disabled";
    }
    
    if (!isGpsLocked()) {
        return "CRITICAL: GPS unlocked";
    }
    
    if (!isPpsValid()) {
        return "CRITICAL: PPS invalid";
    }
    
    int sats = getSatelliteCount();
    if (sats < SATELLITE_CRITICAL) {
        return QString("CRITICAL: Low satellites (%1)").arg(sats);
    }
    
    double accuracy = getTimeAccuracy();
    if (accuracy >= ACCURACY_CRITICAL) {
        return "CRITICAL: Time accuracy degraded";
    }
    
    if (sats < SATELLITE_WARNING) {
        return QString("WARNING: Low satellites (%1)").arg(sats);
    }
    
    return QString("%1 - %2 sats, Accuracy: %3 ns")
           .arg(getSyncSource())
           .arg(sats)
           .arg(accuracy, 0, 'f', 0);
}

void TimingSyncSubsystem::onDataUpdate(const QVariantMap& data)
{
    if (data.contains("gpsLocked")) {
        bool locked = data["gpsLocked"].toBool();
        if (!locked) {
            addFault(FaultCode(FAULT_GPS_UNLOCK, "GPS unlocked", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_GPS_UNLOCK);
        }
    }
    
    if (data.contains("satelliteCount")) {
        int sats = data["satelliteCount"].toInt();
        if (sats < SATELLITE_CRITICAL) {
            addFault(FaultCode(FAULT_LOW_SATELLITES, "Low satellite count", 
                              FaultSeverity::WARNING, getId()));
        } else {
            clearFault(FAULT_LOW_SATELLITES);
        }
    }
    
    if (data.contains("ppsValid")) {
        bool valid = data["ppsValid"].toBool();
        if (!valid) {
            addFault(FaultCode(FAULT_PPS_INVALID, "PPS signal invalid", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_PPS_INVALID);
        }
    }
    
    if (data.contains("ocxoStability")) {
        double stability = data["ocxoStability"].toDouble();
        if (stability >= STABILITY_CRITICAL) {
            addFault(FaultCode(FAULT_OCXO_DRIFT, "OCXO frequency drift", 
                              FaultSeverity::WARNING, getId()));
        } else {
            clearFault(FAULT_OCXO_DRIFT);
        }
    }
}

} // namespace RadarRMP
