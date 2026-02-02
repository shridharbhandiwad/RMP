#include "subsystems/PowerSupplySubsystem.h"

namespace RadarRMP {

PowerSupplySubsystem::PowerSupplySubsystem(const QString& id, const QString& name, QObject* parent)
    : RadarSubsystem(id, name, SubsystemType::PowerSupply, parent)
{
    setDescription("Power distribution unit with UPS and battery backup");
    initializeTelemetryParameters();
}

void PowerSupplySubsystem::initializeTelemetryParameters()
{
    TelemetryParameter inputVoltage("inputVoltage", "Input Voltage", "VAC");
    inputVoltage.nominal = 220.0;
    inputVoltage.minValue = 0.0;
    inputVoltage.maxValue = 300.0;
    inputVoltage.warningLow = 200.0;
    inputVoltage.criticalLow = 180.0;
    inputVoltage.warningHigh = 250.0;
    inputVoltage.criticalHigh = 270.0;
    inputVoltage.value = 220.0;
    addTelemetryParameter(inputVoltage);
    
    TelemetryParameter outputVoltage("outputVoltage", "Output Voltage", "VDC");
    outputVoltage.nominal = 48.0;
    outputVoltage.minValue = 0.0;
    outputVoltage.maxValue = 60.0;
    outputVoltage.warningLow = 45.0;
    outputVoltage.criticalLow = 42.0;
    outputVoltage.warningHigh = 52.0;
    outputVoltage.criticalHigh = 55.0;
    outputVoltage.value = 48.0;
    addTelemetryParameter(outputVoltage);
    
    TelemetryParameter current("current", "Current", "A");
    current.nominal = 50.0;
    current.minValue = 0.0;
    current.maxValue = 100.0;
    current.warningHigh = 75.0;
    current.criticalHigh = 90.0;
    current.value = 50.0;
    addTelemetryParameter(current);
    
    TelemetryParameter power("power", "Power", "kW");
    power.nominal = 2.4;
    power.minValue = 0.0;
    power.maxValue = 10.0;
    power.value = 2.4;
    addTelemetryParameter(power);
    
    TelemetryParameter temperature("temperature", "Temperature", "°C");
    temperature.nominal = 35.0;
    temperature.minValue = 0.0;
    temperature.maxValue = 100.0;
    temperature.warningHigh = 50.0;
    temperature.criticalHigh = 65.0;
    temperature.value = 35.0;
    addTelemetryParameter(temperature);
    
    TelemetryParameter batteryLevel("batteryLevel", "Battery Level", "%");
    batteryLevel.nominal = 100.0;
    batteryLevel.minValue = 0.0;
    batteryLevel.maxValue = 100.0;
    batteryLevel.warningLow = 30.0;
    batteryLevel.criticalLow = 10.0;
    batteryLevel.value = 100.0;
    addTelemetryParameter(batteryLevel);
    
    TelemetryParameter onBattery("onBattery", "On Battery", "");
    onBattery.value = false;
    addTelemetryParameter(onBattery);
    
    TelemetryParameter efficiency("efficiency", "Efficiency", "%");
    efficiency.nominal = 95.0;
    efficiency.minValue = 0.0;
    efficiency.maxValue = 100.0;
    efficiency.warningLow = 85.0;
    efficiency.criticalLow = 75.0;
    efficiency.value = 95.0;
    addTelemetryParameter(efficiency);
    
    TelemetryParameter powerFactor("powerFactor", "Power Factor", "");
    powerFactor.nominal = 0.98;
    powerFactor.minValue = 0.0;
    powerFactor.maxValue = 1.0;
    powerFactor.value = 0.98;
    addTelemetryParameter(powerFactor);
    
    TelemetryParameter psuMode("psuMode", "PSU Mode", "");
    psuMode.value = "NORMAL";
    addTelemetryParameter(psuMode);
}

double PowerSupplySubsystem::getInputVoltage() const
{
    return getTelemetryValue("inputVoltage").toDouble();
}

double PowerSupplySubsystem::getOutputVoltage() const
{
    return getTelemetryValue("outputVoltage").toDouble();
}

double PowerSupplySubsystem::getCurrent() const
{
    return getTelemetryValue("current").toDouble();
}

double PowerSupplySubsystem::getPower() const
{
    return getTelemetryValue("power").toDouble();
}

double PowerSupplySubsystem::getTemperature() const
{
    return getTelemetryValue("temperature").toDouble();
}

double PowerSupplySubsystem::getBatteryLevel() const
{
    return getTelemetryValue("batteryLevel").toDouble();
}

bool PowerSupplySubsystem::isOnBattery() const
{
    return getTelemetryValue("onBattery").toBool();
}

double PowerSupplySubsystem::getEfficiency() const
{
    return getTelemetryValue("efficiency").toDouble();
}

double PowerSupplySubsystem::getPowerFactor() const
{
    return getTelemetryValue("powerFactor").toDouble();
}

QString PowerSupplySubsystem::getPsuMode() const
{
    return getTelemetryValue("psuMode").toString();
}

HealthState PowerSupplySubsystem::computeHealthState() const
{
    if (!isEnabled()) {
        return HealthState::UNKNOWN;
    }
    
    double inputV = getInputVoltage();
    double battery = getBatteryLevel();
    double temp = getTemperature();
    
    if (inputV <= INPUT_VOLTAGE_LOW_CRITICAL || inputV >= INPUT_VOLTAGE_HIGH_CRITICAL ||
        temp >= TEMP_CRITICAL || (isOnBattery() && battery <= BATTERY_CRITICAL)) {
        return HealthState::FAIL;
    }
    
    if (inputV <= INPUT_VOLTAGE_LOW_WARNING || inputV >= INPUT_VOLTAGE_HIGH_WARNING ||
        temp >= TEMP_WARNING || (isOnBattery() && battery <= BATTERY_WARNING)) {
        return HealthState::DEGRADED;
    }
    
    if (isOnBattery()) {
        return HealthState::DEGRADED;
    }
    
    if (hasFaults()) {
        return HealthState::DEGRADED;
    }
    
    return HealthState::OK;
}

double PowerSupplySubsystem::computeHealthScore() const
{
    double score = 100.0;
    
    double inputV = getInputVoltage();
    if (inputV <= INPUT_VOLTAGE_LOW_CRITICAL || inputV >= INPUT_VOLTAGE_HIGH_CRITICAL) {
        score -= 35;
    } else if (inputV <= INPUT_VOLTAGE_LOW_WARNING) {
        score -= 15 * (INPUT_VOLTAGE_LOW_WARNING - inputV) / (INPUT_VOLTAGE_LOW_WARNING - INPUT_VOLTAGE_LOW_CRITICAL);
    } else if (inputV >= INPUT_VOLTAGE_HIGH_WARNING) {
        score -= 15 * (inputV - INPUT_VOLTAGE_HIGH_WARNING) / (INPUT_VOLTAGE_HIGH_CRITICAL - INPUT_VOLTAGE_HIGH_WARNING);
    }
    
    double temp = getTemperature();
    if (temp >= TEMP_CRITICAL) {
        score -= 30;
    } else if (temp >= TEMP_WARNING) {
        score -= 15 * (temp - TEMP_WARNING) / (TEMP_CRITICAL - TEMP_WARNING);
    }
    
    if (isOnBattery()) {
        double battery = getBatteryLevel();
        score -= 10;  // Penalty for running on battery
        if (battery <= BATTERY_CRITICAL) {
            score -= 25;
        } else if (battery <= BATTERY_WARNING) {
            score -= 15 * (BATTERY_WARNING - battery) / (BATTERY_WARNING - BATTERY_CRITICAL);
        }
    }
    
    score -= getFaultCount() * 5;
    
    return qMax(0.0, qMin(100.0, score));
}

QString PowerSupplySubsystem::computeStatusMessage() const
{
    if (!isEnabled()) {
        return "Power Supply disabled";
    }
    
    double inputV = getInputVoltage();
    if (inputV <= INPUT_VOLTAGE_LOW_CRITICAL) {
        return "CRITICAL: Input voltage low";
    }
    if (inputV >= INPUT_VOLTAGE_HIGH_CRITICAL) {
        return "CRITICAL: Input voltage high";
    }
    
    if (isOnBattery()) {
        double battery = getBatteryLevel();
        if (battery <= BATTERY_CRITICAL) {
            return QString("CRITICAL: Battery low (%1%)").arg(battery, 0, 'f', 0);
        }
        return QString("Running on battery - %1% remaining").arg(battery, 0, 'f', 0);
    }
    
    if (inputV <= INPUT_VOLTAGE_LOW_WARNING) {
        return "WARNING: Input voltage low";
    }
    if (inputV >= INPUT_VOLTAGE_HIGH_WARNING) {
        return "WARNING: Input voltage high";
    }
    
    return QString("Normal - %1 kW @ %2% efficiency")
           .arg(getPower(), 0, 'f', 1)
           .arg(getEfficiency(), 0, 'f', 0);
}

void PowerSupplySubsystem::onDataUpdate(const QVariantMap& data)
{
    if (data.contains("inputVoltage")) {
        double v = data["inputVoltage"].toDouble();
        if (v <= INPUT_VOLTAGE_LOW_CRITICAL) {
            addFault(FaultCode(FAULT_INPUT_LOW, "Input voltage low", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_INPUT_LOW);
        }
        if (v >= INPUT_VOLTAGE_HIGH_CRITICAL) {
            addFault(FaultCode(FAULT_INPUT_HIGH, "Input voltage high", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_INPUT_HIGH);
        }
    }
    
    if (data.contains("batteryLevel") && data.contains("onBattery")) {
        double battery = data["batteryLevel"].toDouble();
        bool onBattery = data["onBattery"].toBool();
        if (onBattery && battery <= BATTERY_CRITICAL) {
            addFault(FaultCode(FAULT_BATTERY_LOW, "Battery critically low", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_BATTERY_LOW);
        }
    }
    
    if (data.contains("temperature")) {
        double temp = data["temperature"].toDouble();
        if (temp >= TEMP_CRITICAL) {
            addFault(FaultCode(FAULT_OVERTEMP, "PSU overtemperature", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_OVERTEMP);
        }
    }
}

} // namespace RadarRMP
