#include "subsystems/ReceiverSubsystem.h"

namespace RadarRMP {

ReceiverSubsystem::ReceiverSubsystem(const QString& id, const QString& name, QObject* parent)
    : RadarSubsystem(id, name, SubsystemType::Receiver, parent)
{
    setDescription("Low-noise RF receiver with LNA, AGC, and digital conversion");
    initializeTelemetryParameters();
}

void ReceiverSubsystem::initializeTelemetryParameters()
{
    TelemetryParameter noiseFigure("noiseFigure", "Noise Figure", "dB");
    noiseFigure.nominal = 2.5;
    noiseFigure.minValue = 0.0;
    noiseFigure.maxValue = 15.0;
    noiseFigure.warningHigh = 4.0;
    noiseFigure.criticalHigh = 6.0;
    noiseFigure.value = 2.5;
    addTelemetryParameter(noiseFigure);
    
    TelemetryParameter gain("gain", "Gain", "dB");
    gain.nominal = 30.0;
    gain.minValue = 0.0;
    gain.maxValue = 50.0;
    gain.warningLow = 25.0;
    gain.criticalLow = 20.0;
    gain.value = 30.0;
    addTelemetryParameter(gain);
    
    TelemetryParameter agcLevel("agcLevel", "AGC Level", "dB");
    agcLevel.nominal = 0.0;
    agcLevel.minValue = -30.0;
    agcLevel.maxValue = 30.0;
    agcLevel.value = 0.0;
    addTelemetryParameter(agcLevel);
    
    TelemetryParameter temperature("temperature", "Temperature", "°C");
    temperature.nominal = 35.0;
    temperature.minValue = 0.0;
    temperature.maxValue = 100.0;
    temperature.warningHigh = 55.0;
    temperature.criticalHigh = 70.0;
    temperature.value = 35.0;
    addTelemetryParameter(temperature);
    
    TelemetryParameter signalLevel("signalLevel", "Signal Level", "dBm");
    signalLevel.nominal = -60.0;
    signalLevel.minValue = -120.0;
    signalLevel.maxValue = 0.0;
    signalLevel.value = -60.0;
    addTelemetryParameter(signalLevel);
    
    TelemetryParameter lnaEnabled("lnaEnabled", "LNA Enabled", "");
    lnaEnabled.value = true;
    addTelemetryParameter(lnaEnabled);
    
    TelemetryParameter dynamicRange("dynamicRange", "Dynamic Range", "dB");
    dynamicRange.nominal = 80.0;
    dynamicRange.minValue = 0.0;
    dynamicRange.maxValue = 120.0;
    dynamicRange.value = 80.0;
    addTelemetryParameter(dynamicRange);
    
    TelemetryParameter sensitivity("sensitivity", "Sensitivity", "dBm");
    sensitivity.nominal = -110.0;
    sensitivity.minValue = -130.0;
    sensitivity.maxValue = -50.0;
    sensitivity.value = -110.0;
    addTelemetryParameter(sensitivity);
}

double ReceiverSubsystem::getNoiseFigure() const
{
    return getTelemetryValue("noiseFigure").toDouble();
}

double ReceiverSubsystem::getGain() const
{
    return getTelemetryValue("gain").toDouble();
}

double ReceiverSubsystem::getAgcLevel() const
{
    return getTelemetryValue("agcLevel").toDouble();
}

double ReceiverSubsystem::getTemperature() const
{
    return getTelemetryValue("temperature").toDouble();
}

double ReceiverSubsystem::getSignalLevel() const
{
    return getTelemetryValue("signalLevel").toDouble();
}

bool ReceiverSubsystem::isLnaEnabled() const
{
    return getTelemetryValue("lnaEnabled").toBool();
}

double ReceiverSubsystem::getDynamicRange() const
{
    return getTelemetryValue("dynamicRange").toDouble();
}

double ReceiverSubsystem::getSensitivity() const
{
    return getTelemetryValue("sensitivity").toDouble();
}

HealthState ReceiverSubsystem::computeHealthState() const
{
    if (!isEnabled()) {
        return HealthState::UNKNOWN;
    }
    
    double nf = getNoiseFigure();
    double gain = getGain();
    double temp = getTemperature();
    
    if (nf >= NOISE_FIGURE_CRITICAL || gain <= GAIN_CRITICAL_LOW || temp >= TEMP_CRITICAL) {
        return HealthState::FAIL;
    }
    
    if (nf >= NOISE_FIGURE_WARNING || gain <= GAIN_WARNING_LOW || temp >= TEMP_WARNING) {
        return HealthState::DEGRADED;
    }
    
    if (hasFaults()) {
        return HealthState::DEGRADED;
    }
    
    return HealthState::OK;
}

double ReceiverSubsystem::computeHealthScore() const
{
    double score = 100.0;
    
    double nf = getNoiseFigure();
    if (nf >= NOISE_FIGURE_CRITICAL) {
        score -= 35;
    } else if (nf >= NOISE_FIGURE_WARNING) {
        score -= 15 * (nf - NOISE_FIGURE_WARNING) / (NOISE_FIGURE_CRITICAL - NOISE_FIGURE_WARNING);
    }
    
    double gain = getGain();
    if (gain <= GAIN_CRITICAL_LOW) {
        score -= 35;
    } else if (gain <= GAIN_WARNING_LOW) {
        score -= 15 * (GAIN_WARNING_LOW - gain) / (GAIN_WARNING_LOW - GAIN_CRITICAL_LOW);
    }
    
    double temp = getTemperature();
    if (temp >= TEMP_CRITICAL) {
        score -= 30;
    } else if (temp >= TEMP_WARNING) {
        score -= 15 * (temp - TEMP_WARNING) / (TEMP_CRITICAL - TEMP_WARNING);
    }
    
    score -= getFaultCount() * 5;
    
    return qMax(0.0, qMin(100.0, score));
}

QString ReceiverSubsystem::computeStatusMessage() const
{
    if (!isEnabled()) {
        return "Receiver disabled";
    }
    
    double nf = getNoiseFigure();
    if (nf >= NOISE_FIGURE_CRITICAL) {
        return "CRITICAL: High noise figure";
    }
    
    double gain = getGain();
    if (gain <= GAIN_CRITICAL_LOW) {
        return "CRITICAL: Low gain - LNA failure";
    }
    
    if (nf >= NOISE_FIGURE_WARNING) {
        return "WARNING: Elevated noise figure";
    }
    
    if (gain <= GAIN_WARNING_LOW) {
        return "WARNING: Reduced gain";
    }
    
    return QString("Receiving - NF: %1 dB, Gain: %2 dB")
           .arg(nf, 0, 'f', 1)
           .arg(gain, 0, 'f', 1);
}

void ReceiverSubsystem::onDataUpdate(const QVariantMap& data)
{
    if (data.contains("noiseFigure")) {
        double nf = data["noiseFigure"].toDouble();
        if (nf >= NOISE_FIGURE_CRITICAL) {
            addFault(FaultCode(FAULT_NOISE_FIGURE_HIGH, "High noise figure", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_NOISE_FIGURE_HIGH);
        }
    }
    
    if (data.contains("gain")) {
        double gain = data["gain"].toDouble();
        if (gain <= GAIN_CRITICAL_LOW) {
            addFault(FaultCode(FAULT_GAIN_LOW, "Low receiver gain", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_GAIN_LOW);
        }
    }
    
    if (data.contains("temperature")) {
        double temp = data["temperature"].toDouble();
        if (temp >= TEMP_CRITICAL) {
            addFault(FaultCode(FAULT_OVERTEMP, "Receiver overtemperature", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_OVERTEMP);
        }
    }
}

} // namespace RadarRMP
