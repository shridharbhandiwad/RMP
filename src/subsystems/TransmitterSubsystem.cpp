#include "subsystems/TransmitterSubsystem.h"

namespace RadarRMP {

TransmitterSubsystem::TransmitterSubsystem(const QString& id, const QString& name, QObject* parent)
    : RadarSubsystem(id, name, SubsystemType::Transmitter, parent)
{
    setDescription("High-power RF transmitter unit with modulator and HV power supply");
    initializeTelemetryParameters();
}

void TransmitterSubsystem::initializeTelemetryParameters()
{
    // RF Power
    TelemetryParameter rfPower("rfPower", "RF Output Power", "kW");
    rfPower.nominal = 100.0;
    rfPower.minValue = 0.0;
    rfPower.maxValue = 150.0;
    rfPower.warningLow = 80.0;
    rfPower.criticalLow = 50.0;
    rfPower.value = 100.0;
    addTelemetryParameter(rfPower);
    
    // VSWR
    TelemetryParameter vswr("vswr", "VSWR", "");
    vswr.nominal = 1.2;
    vswr.minValue = 1.0;
    vswr.maxValue = 5.0;
    vswr.warningHigh = 1.5;
    vswr.criticalHigh = 2.0;
    vswr.value = 1.2;
    addTelemetryParameter(vswr);
    
    // Temperature
    TelemetryParameter temp("temperature", "Temperature", "°C");
    temp.nominal = 45.0;
    temp.minValue = 0.0;
    temp.maxValue = 100.0;
    temp.warningHigh = 60.0;
    temp.criticalHigh = 80.0;
    temp.value = 45.0;
    addTelemetryParameter(temp);
    
    // Duty Cycle
    TelemetryParameter dutyCycle("dutyCycle", "Duty Cycle", "%");
    dutyCycle.nominal = 10.0;
    dutyCycle.minValue = 0.0;
    dutyCycle.maxValue = 100.0;
    dutyCycle.warningHigh = 20.0;
    dutyCycle.criticalHigh = 30.0;
    dutyCycle.value = 10.0;
    addTelemetryParameter(dutyCycle);
    
    // HV Voltage
    TelemetryParameter hvVoltage("hvVoltage", "HV Voltage", "kV");
    hvVoltage.nominal = 25.0;
    hvVoltage.minValue = 0.0;
    hvVoltage.maxValue = 35.0;
    hvVoltage.warningLow = 22.5;
    hvVoltage.criticalLow = 20.0;
    hvVoltage.warningHigh = 27.5;
    hvVoltage.criticalHigh = 30.0;
    hvVoltage.value = 25.0;
    addTelemetryParameter(hvVoltage);
    
    // HV Enabled
    TelemetryParameter hvEnabled("hvEnabled", "HV Enabled", "");
    hvEnabled.value = true;
    addTelemetryParameter(hvEnabled);
    
    // TX Mode
    TelemetryParameter txMode("txMode", "TX Mode", "");
    txMode.value = "NORMAL";
    addTelemetryParameter(txMode);
    
    // Pulse Width
    TelemetryParameter pulseWidth("pulseWidth", "Pulse Width", "µs");
    pulseWidth.nominal = 10.0;
    pulseWidth.minValue = 1.0;
    pulseWidth.maxValue = 100.0;
    pulseWidth.value = 10.0;
    addTelemetryParameter(pulseWidth);
    
    // PRF
    TelemetryParameter prf("prf", "PRF", "Hz");
    prf.nominal = 1000.0;
    prf.minValue = 100.0;
    prf.maxValue = 5000.0;
    prf.value = 1000.0;
    addTelemetryParameter(prf);
}

double TransmitterSubsystem::getRfPower() const
{
    return getTelemetryValue("rfPower").toDouble();
}

double TransmitterSubsystem::getVswr() const
{
    return getTelemetryValue("vswr").toDouble();
}

double TransmitterSubsystem::getTemperature() const
{
    return getTelemetryValue("temperature").toDouble();
}

double TransmitterSubsystem::getDutyCycle() const
{
    return getTelemetryValue("dutyCycle").toDouble();
}

double TransmitterSubsystem::getHvVoltage() const
{
    return getTelemetryValue("hvVoltage").toDouble();
}

bool TransmitterSubsystem::isHvEnabled() const
{
    return getTelemetryValue("hvEnabled").toBool();
}

QString TransmitterSubsystem::getTxMode() const
{
    return getTelemetryValue("txMode").toString();
}

double TransmitterSubsystem::getPulseWidth() const
{
    return getTelemetryValue("pulseWidth").toDouble();
}

double TransmitterSubsystem::getPrf() const
{
    return getTelemetryValue("prf").toDouble();
}

HealthState TransmitterSubsystem::computeHealthState() const
{
    if (!isEnabled()) {
        return HealthState::UNKNOWN;
    }
    
    // Check for critical conditions
    double temp = getTemperature();
    double vswr = getVswr();
    double rfPower = getRfPower();
    double hvVoltage = getHvVoltage();
    
    // Critical failures
    if (temp >= TEMP_CRITICAL) {
        return HealthState::FAIL;
    }
    if (vswr >= VSWR_CRITICAL) {
        return HealthState::FAIL;
    }
    if (rfPower <= RF_POWER_CRITICAL_LOW) {
        return HealthState::FAIL;
    }
    if (hvVoltage <= HV_CRITICAL_LOW * 0.25) {  // 80% of nominal 25kV
        return HealthState::FAIL;
    }
    
    // Check for degraded conditions
    if (temp >= TEMP_WARNING) {
        return HealthState::DEGRADED;
    }
    if (vswr >= VSWR_WARNING) {
        return HealthState::DEGRADED;
    }
    if (rfPower <= RF_POWER_WARNING_LOW) {
        return HealthState::DEGRADED;
    }
    if (hvVoltage <= HV_WARNING_LOW * 0.25) {
        return HealthState::DEGRADED;
    }
    
    // Check for active faults
    if (hasFaults()) {
        return HealthState::DEGRADED;
    }
    
    return HealthState::OK;
}

double TransmitterSubsystem::computeHealthScore() const
{
    double score = 100.0;
    
    // Temperature score
    double temp = getTemperature();
    if (temp >= TEMP_CRITICAL) {
        score -= 40;
    } else if (temp >= TEMP_WARNING) {
        score -= 20 * (temp - TEMP_WARNING) / (TEMP_CRITICAL - TEMP_WARNING);
    }
    
    // VSWR score
    double vswr = getVswr();
    if (vswr >= VSWR_CRITICAL) {
        score -= 30;
    } else if (vswr >= VSWR_WARNING) {
        score -= 15 * (vswr - VSWR_WARNING) / (VSWR_CRITICAL - VSWR_WARNING);
    }
    
    // RF Power score
    double rfPower = getRfPower();
    if (rfPower <= RF_POWER_CRITICAL_LOW) {
        score -= 30;
    } else if (rfPower <= RF_POWER_WARNING_LOW) {
        score -= 15 * (RF_POWER_WARNING_LOW - rfPower) / (RF_POWER_WARNING_LOW - RF_POWER_CRITICAL_LOW);
    }
    
    // Fault penalty
    score -= getFaultCount() * 5;
    
    return qMax(0.0, qMin(100.0, score));
}

QString TransmitterSubsystem::computeStatusMessage() const
{
    if (!isEnabled()) {
        return "Transmitter disabled";
    }
    
    if (!isHvEnabled()) {
        return "HV off - Standby mode";
    }
    
    double temp = getTemperature();
    if (temp >= TEMP_CRITICAL) {
        return "CRITICAL: Over temperature";
    }
    
    double vswr = getVswr();
    if (vswr >= VSWR_CRITICAL) {
        return "CRITICAL: High VSWR - Check antenna";
    }
    
    double rfPower = getRfPower();
    if (rfPower <= RF_POWER_CRITICAL_LOW) {
        return "CRITICAL: Low RF output";
    }
    
    if (temp >= TEMP_WARNING) {
        return "WARNING: Elevated temperature";
    }
    
    if (vswr >= VSWR_WARNING) {
        return "WARNING: VSWR above normal";
    }
    
    if (rfPower <= RF_POWER_WARNING_LOW) {
        return "WARNING: RF power below nominal";
    }
    
    return QString("Transmitting - %1 kW @ %2 Hz PRF")
           .arg(rfPower, 0, 'f', 1)
           .arg(getPrf(), 0, 'f', 0);
}

void TransmitterSubsystem::onDataUpdate(const QVariantMap& data)
{
    // Check for fault conditions after data update
    
    // VSWR fault
    if (data.contains("vswr")) {
        double vswr = data["vswr"].toDouble();
        if (vswr >= VSWR_CRITICAL) {
            addFault(FaultCode(FAULT_VSWR_HIGH, "High VSWR detected", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_VSWR_HIGH);
        }
    }
    
    // Temperature fault
    if (data.contains("temperature")) {
        double temp = data["temperature"].toDouble();
        if (temp >= TEMP_CRITICAL) {
            addFault(FaultCode(FAULT_OVERTEMP, "Transmitter overtemperature", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_OVERTEMP);
        }
    }
    
    // RF Power fault
    if (data.contains("rfPower")) {
        double power = data["rfPower"].toDouble();
        if (power <= RF_POWER_CRITICAL_LOW && isHvEnabled()) {
            addFault(FaultCode(FAULT_RF_POWER_LOW, "Low RF output power", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_RF_POWER_LOW);
        }
    }
}

} // namespace RadarRMP
