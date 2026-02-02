#include "subsystems/CoolingSubsystem.h"

namespace RadarRMP {

CoolingSubsystem::CoolingSubsystem(const QString& id, const QString& name, QObject* parent)
    : RadarSubsystem(id, name, SubsystemType::Cooling, parent)
{
    setDescription("Thermal management system with liquid cooling and HVAC");
    initializeTelemetryParameters();
}

void CoolingSubsystem::initializeTelemetryParameters()
{
    TelemetryParameter coolantTemp("coolantTemp", "Coolant Temperature", "°C");
    coolantTemp.nominal = 25.0;
    coolantTemp.minValue = 0.0;
    coolantTemp.maxValue = 80.0;
    coolantTemp.warningHigh = 45.0;
    coolantTemp.criticalHigh = 55.0;
    coolantTemp.value = 25.0;
    addTelemetryParameter(coolantTemp);
    
    TelemetryParameter coolantFlow("coolantFlow", "Coolant Flow", "L/min");
    coolantFlow.nominal = 20.0;
    coolantFlow.minValue = 0.0;
    coolantFlow.maxValue = 50.0;
    coolantFlow.warningLow = 14.0;
    coolantFlow.criticalLow = 10.0;
    coolantFlow.value = 20.0;
    addTelemetryParameter(coolantFlow);
    
    TelemetryParameter ambientTemp("ambientTemp", "Ambient Temperature", "°C");
    ambientTemp.nominal = 25.0;
    ambientTemp.minValue = -20.0;
    ambientTemp.maxValue = 60.0;
    ambientTemp.warningHigh = 40.0;
    ambientTemp.criticalHigh = 50.0;
    ambientTemp.value = 25.0;
    addTelemetryParameter(ambientTemp);
    
    TelemetryParameter fanSpeed("fanSpeed", "Fan Speed", "%");
    fanSpeed.nominal = 50.0;
    fanSpeed.minValue = 0.0;
    fanSpeed.maxValue = 100.0;
    fanSpeed.value = 50.0;
    addTelemetryParameter(fanSpeed);
    
    TelemetryParameter heatLoad("heatLoad", "Heat Load", "kW");
    heatLoad.nominal = 5.0;
    heatLoad.minValue = 0.0;
    heatLoad.maxValue = 20.0;
    heatLoad.value = 5.0;
    addTelemetryParameter(heatLoad);
    
    TelemetryParameter efficiency("efficiency", "Efficiency", "%");
    efficiency.nominal = 90.0;
    efficiency.minValue = 0.0;
    efficiency.maxValue = 100.0;
    efficiency.warningLow = 70.0;
    efficiency.criticalLow = 50.0;
    efficiency.value = 90.0;
    addTelemetryParameter(efficiency);
    
    TelemetryParameter coolingMode("coolingMode", "Cooling Mode", "");
    coolingMode.value = "AUTO";
    addTelemetryParameter(coolingMode);
    
    TelemetryParameter compressorPressure("compressorPressure", "Compressor Pressure", "bar");
    compressorPressure.nominal = 15.0;
    compressorPressure.minValue = 0.0;
    compressorPressure.maxValue = 30.0;
    compressorPressure.value = 15.0;
    addTelemetryParameter(compressorPressure);
    
    TelemetryParameter compressorRunning("compressorRunning", "Compressor Running", "");
    compressorRunning.value = true;
    addTelemetryParameter(compressorRunning);
}

double CoolingSubsystem::getCoolantTemp() const
{
    return getTelemetryValue("coolantTemp").toDouble();
}

double CoolingSubsystem::getCoolantFlow() const
{
    return getTelemetryValue("coolantFlow").toDouble();
}

double CoolingSubsystem::getAmbientTemp() const
{
    return getTelemetryValue("ambientTemp").toDouble();
}

double CoolingSubsystem::getFanSpeed() const
{
    return getTelemetryValue("fanSpeed").toDouble();
}

double CoolingSubsystem::getHeatLoad() const
{
    return getTelemetryValue("heatLoad").toDouble();
}

double CoolingSubsystem::getEfficiency() const
{
    return getTelemetryValue("efficiency").toDouble();
}

QString CoolingSubsystem::getCoolingMode() const
{
    return getTelemetryValue("coolingMode").toString();
}

double CoolingSubsystem::getCompressorPressure() const
{
    return getTelemetryValue("compressorPressure").toDouble();
}

bool CoolingSubsystem::isCompressorRunning() const
{
    return getTelemetryValue("compressorRunning").toBool();
}

HealthState CoolingSubsystem::computeHealthState() const
{
    if (!isEnabled()) {
        return HealthState::UNKNOWN;
    }
    
    double coolantT = getCoolantTemp();
    double flow = getCoolantFlow();
    double ambient = getAmbientTemp();
    double eff = getEfficiency();
    
    if (coolantT >= COOLANT_TEMP_CRITICAL || flow <= FLOW_CRITICAL_LOW * 0.2 ||
        ambient >= AMBIENT_CRITICAL || eff <= EFFICIENCY_CRITICAL) {
        return HealthState::FAIL;
    }
    
    if (coolantT >= COOLANT_TEMP_WARNING || flow <= FLOW_WARNING_LOW * 0.2 ||
        ambient >= AMBIENT_WARNING || eff <= EFFICIENCY_WARNING) {
        return HealthState::DEGRADED;
    }
    
    if (hasFaults()) {
        return HealthState::DEGRADED;
    }
    
    return HealthState::OK;
}

double CoolingSubsystem::computeHealthScore() const
{
    double score = 100.0;
    
    double coolantT = getCoolantTemp();
    if (coolantT >= COOLANT_TEMP_CRITICAL) {
        score -= 35;
    } else if (coolantT >= COOLANT_TEMP_WARNING) {
        score -= 15 * (coolantT - COOLANT_TEMP_WARNING) / (COOLANT_TEMP_CRITICAL - COOLANT_TEMP_WARNING);
    }
    
    double flowPct = getCoolantFlow() / 20.0 * 100.0;  // Relative to nominal
    if (flowPct <= FLOW_CRITICAL_LOW) {
        score -= 30;
    } else if (flowPct <= FLOW_WARNING_LOW) {
        score -= 15 * (FLOW_WARNING_LOW - flowPct) / (FLOW_WARNING_LOW - FLOW_CRITICAL_LOW);
    }
    
    double ambient = getAmbientTemp();
    if (ambient >= AMBIENT_CRITICAL) {
        score -= 20;
    } else if (ambient >= AMBIENT_WARNING) {
        score -= 10 * (ambient - AMBIENT_WARNING) / (AMBIENT_CRITICAL - AMBIENT_WARNING);
    }
    
    double eff = getEfficiency();
    if (eff <= EFFICIENCY_CRITICAL) {
        score -= 15;
    } else if (eff <= EFFICIENCY_WARNING) {
        score -= 8 * (EFFICIENCY_WARNING - eff) / (EFFICIENCY_WARNING - EFFICIENCY_CRITICAL);
    }
    
    score -= getFaultCount() * 5;
    
    return qMax(0.0, qMin(100.0, score));
}

QString CoolingSubsystem::computeStatusMessage() const
{
    if (!isEnabled()) {
        return "Cooling System disabled";
    }
    
    double coolantT = getCoolantTemp();
    if (coolantT >= COOLANT_TEMP_CRITICAL) {
        return "CRITICAL: Coolant overtemperature";
    }
    
    double flowPct = getCoolantFlow() / 20.0 * 100.0;
    if (flowPct <= FLOW_CRITICAL_LOW) {
        return "CRITICAL: Low coolant flow";
    }
    
    if (coolantT >= COOLANT_TEMP_WARNING) {
        return "WARNING: Elevated coolant temperature";
    }
    
    if (flowPct <= FLOW_WARNING_LOW) {
        return "WARNING: Reduced coolant flow";
    }
    
    return QString("%1 Mode - Coolant: %2°C, Flow: %3 L/min")
           .arg(getCoolingMode())
           .arg(coolantT, 0, 'f', 1)
           .arg(getCoolantFlow(), 0, 'f', 1);
}

void CoolingSubsystem::onDataUpdate(const QVariantMap& data)
{
    if (data.contains("coolantTemp")) {
        double temp = data["coolantTemp"].toDouble();
        if (temp >= COOLANT_TEMP_CRITICAL) {
            addFault(FaultCode(FAULT_COOLANT_TEMP_HIGH, "Coolant overtemperature", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_COOLANT_TEMP_HIGH);
        }
    }
    
    if (data.contains("coolantFlow")) {
        double flow = data["coolantFlow"].toDouble();
        double flowPct = flow / 20.0 * 100.0;
        if (flowPct <= FLOW_CRITICAL_LOW) {
            addFault(FaultCode(FAULT_COOLANT_FLOW_LOW, "Low coolant flow", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_COOLANT_FLOW_LOW);
        }
    }
    
    if (data.contains("efficiency")) {
        double eff = data["efficiency"].toDouble();
        if (eff <= EFFICIENCY_CRITICAL) {
            addFault(FaultCode(FAULT_EFFICIENCY_LOW, "Cooling efficiency degraded", 
                              FaultSeverity::WARNING, getId()));
        } else {
            clearFault(FAULT_EFFICIENCY_LOW);
        }
    }
}

} // namespace RadarRMP
