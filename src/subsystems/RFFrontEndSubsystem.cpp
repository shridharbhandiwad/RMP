#include "subsystems/RFFrontEndSubsystem.h"

namespace RadarRMP {

RFFrontEndSubsystem::RFFrontEndSubsystem(const QString& id, const QString& name, QObject* parent)
    : RadarSubsystem(id, name, SubsystemType::RFFrontEnd, parent)
{
    setDescription("RF front-end with frequency synthesizer, mixers, and T/R switching");
    initializeTelemetryParameters();
}

void RFFrontEndSubsystem::initializeTelemetryParameters()
{
    TelemetryParameter frequency("frequency", "Frequency", "GHz");
    frequency.nominal = 9.5;
    frequency.minValue = 9.0;
    frequency.maxValue = 10.0;
    frequency.value = 9.5;
    addTelemetryParameter(frequency);
    
    TelemetryParameter phaseLock("phaseLock", "Phase Lock", "");
    phaseLock.nominal = 1.0;
    phaseLock.minValue = 0.0;
    phaseLock.maxValue = 1.0;
    phaseLock.warningLow = 0.8;
    phaseLock.criticalLow = 0.5;
    phaseLock.value = 1.0;
    addTelemetryParameter(phaseLock);
    
    TelemetryParameter ifLevel("ifLevel", "IF Level", "dBm");
    ifLevel.nominal = -10.0;
    ifLevel.minValue = -40.0;
    ifLevel.maxValue = 10.0;
    ifLevel.value = -10.0;
    addTelemetryParameter(ifLevel);
    
    TelemetryParameter loLevel("loLevel", "LO Level", "dBm");
    loLevel.nominal = 10.0;
    loLevel.minValue = -10.0;
    loLevel.maxValue = 20.0;
    loLevel.warningLow = 5.0;
    loLevel.criticalLow = 0.0;
    loLevel.value = 10.0;
    addTelemetryParameter(loLevel);
    
    TelemetryParameter temperature("temperature", "Temperature", "°C");
    temperature.nominal = 40.0;
    temperature.minValue = 0.0;
    temperature.maxValue = 100.0;
    temperature.warningHigh = 55.0;
    temperature.criticalHigh = 70.0;
    temperature.value = 40.0;
    addTelemetryParameter(temperature);
    
    TelemetryParameter trSwitchOk("trSwitchOk", "T/R Switch", "");
    trSwitchOk.value = true;
    addTelemetryParameter(trSwitchOk);
    
    TelemetryParameter phaseError("phaseError", "Phase Error", "°");
    phaseError.nominal = 1.0;
    phaseError.minValue = 0.0;
    phaseError.maxValue = 180.0;
    phaseError.warningHigh = 5.0;
    phaseError.criticalHigh = 15.0;
    phaseError.value = 1.0;
    addTelemetryParameter(phaseError);
    
    TelemetryParameter amplitudeError("amplitudeError", "Amplitude Error", "dB");
    amplitudeError.nominal = 0.5;
    amplitudeError.minValue = 0.0;
    amplitudeError.maxValue = 10.0;
    amplitudeError.warningHigh = 1.0;
    amplitudeError.criticalHigh = 3.0;
    amplitudeError.value = 0.5;
    addTelemetryParameter(amplitudeError);
}

double RFFrontEndSubsystem::getFrequency() const
{
    return getTelemetryValue("frequency").toDouble();
}

double RFFrontEndSubsystem::getPhaseLock() const
{
    return getTelemetryValue("phaseLock").toDouble();
}

double RFFrontEndSubsystem::getIfLevel() const
{
    return getTelemetryValue("ifLevel").toDouble();
}

double RFFrontEndSubsystem::getLoLevel() const
{
    return getTelemetryValue("loLevel").toDouble();
}

double RFFrontEndSubsystem::getTemperature() const
{
    return getTelemetryValue("temperature").toDouble();
}

bool RFFrontEndSubsystem::isTrSwitchOk() const
{
    return getTelemetryValue("trSwitchOk").toBool();
}

double RFFrontEndSubsystem::getPhaseError() const
{
    return getTelemetryValue("phaseError").toDouble();
}

double RFFrontEndSubsystem::getAmplitudeError() const
{
    return getTelemetryValue("amplitudeError").toDouble();
}

HealthState RFFrontEndSubsystem::computeHealthState() const
{
    if (!isEnabled()) {
        return HealthState::UNKNOWN;
    }
    
    double pll = getPhaseLock();
    double temp = getTemperature();
    
    if (pll <= PHASE_LOCK_CRITICAL || temp >= TEMP_CRITICAL || !isTrSwitchOk()) {
        return HealthState::FAIL;
    }
    
    if (pll <= PHASE_LOCK_WARNING || temp >= TEMP_WARNING) {
        return HealthState::DEGRADED;
    }
    
    if (hasFaults()) {
        return HealthState::DEGRADED;
    }
    
    return HealthState::OK;
}

double RFFrontEndSubsystem::computeHealthScore() const
{
    double score = 100.0;
    
    double pll = getPhaseLock();
    if (pll <= PHASE_LOCK_CRITICAL) {
        score -= 40;
    } else if (pll <= PHASE_LOCK_WARNING) {
        score -= 20 * (PHASE_LOCK_WARNING - pll) / (PHASE_LOCK_WARNING - PHASE_LOCK_CRITICAL);
    }
    
    double temp = getTemperature();
    if (temp >= TEMP_CRITICAL) {
        score -= 30;
    } else if (temp >= TEMP_WARNING) {
        score -= 15 * (temp - TEMP_WARNING) / (TEMP_CRITICAL - TEMP_WARNING);
    }
    
    if (!isTrSwitchOk()) {
        score -= 30;
    }
    
    score -= getFaultCount() * 5;
    
    return qMax(0.0, qMin(100.0, score));
}

QString RFFrontEndSubsystem::computeStatusMessage() const
{
    if (!isEnabled()) {
        return "RF Front-End disabled";
    }
    
    if (getPhaseLock() <= PHASE_LOCK_CRITICAL) {
        return "CRITICAL: PLL unlocked";
    }
    
    if (!isTrSwitchOk()) {
        return "CRITICAL: T/R switch failure";
    }
    
    if (getTemperature() >= TEMP_CRITICAL) {
        return "CRITICAL: Overtemperature";
    }
    
    if (getPhaseLock() <= PHASE_LOCK_WARNING) {
        return "WARNING: PLL marginal";
    }
    
    return QString("Locked - %1 GHz, Phase error: %2°")
           .arg(getFrequency(), 0, 'f', 3)
           .arg(getPhaseError(), 0, 'f', 1);
}

void RFFrontEndSubsystem::onDataUpdate(const QVariantMap& data)
{
    if (data.contains("phaseLock")) {
        double pll = data["phaseLock"].toDouble();
        if (pll <= PHASE_LOCK_CRITICAL) {
            addFault(FaultCode(FAULT_PLL_UNLOCK, "PLL unlocked", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_PLL_UNLOCK);
        }
    }
    
    if (data.contains("trSwitchOk")) {
        bool ok = data["trSwitchOk"].toBool();
        if (!ok) {
            addFault(FaultCode(FAULT_TR_SWITCH, "T/R switch failure", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_TR_SWITCH);
        }
    }
    
    if (data.contains("temperature")) {
        double temp = data["temperature"].toDouble();
        if (temp >= TEMP_CRITICAL) {
            addFault(FaultCode(FAULT_OVERTEMP, "RF overtemperature", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_OVERTEMP);
        }
    }
}

} // namespace RadarRMP
