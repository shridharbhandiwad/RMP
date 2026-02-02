#include "subsystems/AntennaServoSubsystem.h"

namespace RadarRMP {

AntennaServoSubsystem::AntennaServoSubsystem(const QString& id, const QString& name, QObject* parent)
    : RadarSubsystem(id, name, SubsystemType::AntennaServo, parent)
{
    setDescription("Antenna positioning system with azimuth/elevation servo control");
    initializeTelemetryParameters();
}

void AntennaServoSubsystem::initializeTelemetryParameters()
{
    TelemetryParameter azimuth("azimuth", "Azimuth", "°");
    azimuth.nominal = 0.0;
    azimuth.minValue = 0.0;
    azimuth.maxValue = 360.0;
    azimuth.value = 0.0;
    addTelemetryParameter(azimuth);
    
    TelemetryParameter elevation("elevation", "Elevation", "°");
    elevation.nominal = 0.0;
    elevation.minValue = -5.0;
    elevation.maxValue = 90.0;
    elevation.value = 0.0;
    addTelemetryParameter(elevation);
    
    TelemetryParameter rotationRate("rotationRate", "Rotation Rate", "°/s");
    rotationRate.nominal = 30.0;
    rotationRate.minValue = 0.0;
    rotationRate.maxValue = 60.0;
    rotationRate.value = 30.0;
    addTelemetryParameter(rotationRate);
    
    TelemetryParameter motorCurrent("motorCurrent", "Motor Current", "A");
    motorCurrent.nominal = 5.0;
    motorCurrent.minValue = 0.0;
    motorCurrent.maxValue = 20.0;
    motorCurrent.warningHigh = 8.0;
    motorCurrent.criticalHigh = 12.0;
    motorCurrent.value = 5.0;
    addTelemetryParameter(motorCurrent);
    
    TelemetryParameter motorTemperature("motorTemperature", "Motor Temperature", "°C");
    motorTemperature.nominal = 45.0;
    motorTemperature.minValue = 0.0;
    motorTemperature.maxValue = 120.0;
    motorTemperature.warningHigh = 65.0;
    motorTemperature.criticalHigh = 85.0;
    motorTemperature.value = 45.0;
    addTelemetryParameter(motorTemperature);
    
    TelemetryParameter positionError("positionError", "Position Error", "°");
    positionError.nominal = 0.1;
    positionError.minValue = 0.0;
    positionError.maxValue = 10.0;
    positionError.warningHigh = 0.5;
    positionError.criticalHigh = 1.0;
    positionError.value = 0.1;
    addTelemetryParameter(positionError);
    
    TelemetryParameter scanMode("scanMode", "Scan Mode", "");
    scanMode.value = "SEARCH";
    addTelemetryParameter(scanMode);
    
    TelemetryParameter azLimitReached("azLimitReached", "Az Limit", "");
    azLimitReached.value = false;
    addTelemetryParameter(azLimitReached);
    
    TelemetryParameter elLimitReached("elLimitReached", "El Limit", "");
    elLimitReached.value = false;
    addTelemetryParameter(elLimitReached);
}

double AntennaServoSubsystem::getAzimuth() const
{
    return getTelemetryValue("azimuth").toDouble();
}

double AntennaServoSubsystem::getElevation() const
{
    return getTelemetryValue("elevation").toDouble();
}

double AntennaServoSubsystem::getRotationRate() const
{
    return getTelemetryValue("rotationRate").toDouble();
}

double AntennaServoSubsystem::getMotorCurrent() const
{
    return getTelemetryValue("motorCurrent").toDouble();
}

double AntennaServoSubsystem::getMotorTemperature() const
{
    return getTelemetryValue("motorTemperature").toDouble();
}

double AntennaServoSubsystem::getPositionError() const
{
    return getTelemetryValue("positionError").toDouble();
}

QString AntennaServoSubsystem::getScanMode() const
{
    return getTelemetryValue("scanMode").toString();
}

bool AntennaServoSubsystem::isAzLimitReached() const
{
    return getTelemetryValue("azLimitReached").toBool();
}

bool AntennaServoSubsystem::isElLimitReached() const
{
    return getTelemetryValue("elLimitReached").toBool();
}

HealthState AntennaServoSubsystem::computeHealthState() const
{
    if (!isEnabled()) {
        return HealthState::UNKNOWN;
    }
    
    double current = getMotorCurrent();
    double temp = getMotorTemperature();
    double error = getPositionError();
    
    if (current >= CURRENT_CRITICAL || temp >= MOTOR_TEMP_CRITICAL || 
        error >= POSITION_ERROR_CRITICAL) {
        return HealthState::FAIL;
    }
    
    if (current >= CURRENT_WARNING || temp >= MOTOR_TEMP_WARNING || 
        error >= POSITION_ERROR_WARNING) {
        return HealthState::DEGRADED;
    }
    
    if (isAzLimitReached() || isElLimitReached()) {
        return HealthState::DEGRADED;
    }
    
    if (hasFaults()) {
        return HealthState::DEGRADED;
    }
    
    return HealthState::OK;
}

double AntennaServoSubsystem::computeHealthScore() const
{
    double score = 100.0;
    
    double current = getMotorCurrent();
    if (current >= CURRENT_CRITICAL) {
        score -= 35;
    } else if (current >= CURRENT_WARNING) {
        score -= 15 * (current - CURRENT_WARNING) / (CURRENT_CRITICAL - CURRENT_WARNING);
    }
    
    double temp = getMotorTemperature();
    if (temp >= MOTOR_TEMP_CRITICAL) {
        score -= 30;
    } else if (temp >= MOTOR_TEMP_WARNING) {
        score -= 15 * (temp - MOTOR_TEMP_WARNING) / (MOTOR_TEMP_CRITICAL - MOTOR_TEMP_WARNING);
    }
    
    double error = getPositionError();
    if (error >= POSITION_ERROR_CRITICAL) {
        score -= 25;
    } else if (error >= POSITION_ERROR_WARNING) {
        score -= 10 * (error - POSITION_ERROR_WARNING) / (POSITION_ERROR_CRITICAL - POSITION_ERROR_WARNING);
    }
    
    if (isAzLimitReached() || isElLimitReached()) {
        score -= 10;
    }
    
    score -= getFaultCount() * 5;
    
    return qMax(0.0, qMin(100.0, score));
}

QString AntennaServoSubsystem::computeStatusMessage() const
{
    if (!isEnabled()) {
        return "Antenna servo disabled";
    }
    
    if (getMotorCurrent() >= CURRENT_CRITICAL) {
        return "CRITICAL: Motor overcurrent";
    }
    
    if (getMotorTemperature() >= MOTOR_TEMP_CRITICAL) {
        return "CRITICAL: Motor overheating";
    }
    
    if (getPositionError() >= POSITION_ERROR_CRITICAL) {
        return "CRITICAL: Position servo error";
    }
    
    if (isAzLimitReached()) {
        return "WARNING: Azimuth limit reached";
    }
    
    if (isElLimitReached()) {
        return "WARNING: Elevation limit reached";
    }
    
    return QString("%1 - Az: %2°, El: %3°")
           .arg(getScanMode())
           .arg(getAzimuth(), 0, 'f', 1)
           .arg(getElevation(), 0, 'f', 1);
}

void AntennaServoSubsystem::onDataUpdate(const QVariantMap& data)
{
    if (data.contains("motorCurrent")) {
        double current = data["motorCurrent"].toDouble();
        if (current >= CURRENT_CRITICAL) {
            addFault(FaultCode(FAULT_MOTOR_OVERCURRENT, "Motor overcurrent", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_MOTOR_OVERCURRENT);
        }
    }
    
    if (data.contains("motorTemperature")) {
        double temp = data["motorTemperature"].toDouble();
        if (temp >= MOTOR_TEMP_CRITICAL) {
            addFault(FaultCode(FAULT_MOTOR_OVERTEMP, "Motor overtemperature", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_MOTOR_OVERTEMP);
        }
    }
    
    if (data.contains("positionError")) {
        double error = data["positionError"].toDouble();
        if (error >= POSITION_ERROR_CRITICAL) {
            addFault(FaultCode(FAULT_POSITION_ERROR, "Position servo error", 
                              FaultSeverity::CRITICAL, getId()));
        } else {
            clearFault(FAULT_POSITION_ERROR);
        }
    }
}

} // namespace RadarRMP
