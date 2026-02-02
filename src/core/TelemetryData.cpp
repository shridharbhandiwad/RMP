#include "core/TelemetryData.h"

namespace RadarRMP {

TelemetryData::TelemetryData(QObject* parent)
    : QObject(parent)
    , m_lastUpdate(QDateTime::currentDateTime())
{
}

void TelemetryData::addParameter(const TelemetryParameter& param)
{
    m_parameters[param.name] = param;
    m_lastUpdate = QDateTime::currentDateTime();
    emit dataChanged();
}

void TelemetryData::removeParameter(const QString& name)
{
    if (m_parameters.remove(name) > 0) {
        emit dataChanged();
    }
}

bool TelemetryData::hasParameter(const QString& name) const
{
    return m_parameters.contains(name);
}

TelemetryParameter TelemetryData::getParameter(const QString& name) const
{
    return m_parameters.value(name);
}

QStringList TelemetryData::getParameterNames() const
{
    return m_parameters.keys();
}

QVariant TelemetryData::getValue(const QString& name) const
{
    if (m_parameters.contains(name)) {
        return m_parameters[name].value;
    }
    return QVariant();
}

void TelemetryData::setValue(const QString& name, const QVariant& value)
{
    if (!m_parameters.contains(name)) {
        return;
    }
    
    TelemetryParameter& param = m_parameters[name];
    QVariant oldValue = param.value;
    param.value = value;
    param.timestamp = QDateTime::currentDateTime();
    m_lastUpdate = param.timestamp;
    
    // Check thresholds
    if (value.canConvert<double>()) {
        double v = value.toDouble();
        
        if (param.criticalLow.isValid() && v < param.criticalLow.toDouble()) {
            emit thresholdExceeded(name, "criticalLow");
        } else if (param.criticalHigh.isValid() && v > param.criticalHigh.toDouble()) {
            emit thresholdExceeded(name, "criticalHigh");
        } else if (param.warningLow.isValid() && v < param.warningLow.toDouble()) {
            emit thresholdExceeded(name, "warningLow");
        } else if (param.warningHigh.isValid() && v > param.warningHigh.toDouble()) {
            emit thresholdExceeded(name, "warningHigh");
        }
    }
    
    emit parameterChanged(name, value);
    emit dataChanged();
}

void TelemetryData::setValues(const QVariantMap& values)
{
    for (auto it = values.begin(); it != values.end(); ++it) {
        if (m_parameters.contains(it.key())) {
            TelemetryParameter& param = m_parameters[it.key()];
            param.value = it.value();
            param.timestamp = QDateTime::currentDateTime();
        }
    }
    m_lastUpdate = QDateTime::currentDateTime();
    emit dataChanged();
}

bool TelemetryData::isWithinLimits(const QString& name) const
{
    if (!m_parameters.contains(name)) {
        return true;
    }
    
    const TelemetryParameter& param = m_parameters[name];
    if (!param.value.canConvert<double>()) {
        return true;
    }
    
    double v = param.value.toDouble();
    
    if (param.minValue.isValid() && v < param.minValue.toDouble()) {
        return false;
    }
    if (param.maxValue.isValid() && v > param.maxValue.toDouble()) {
        return false;
    }
    
    return true;
}

bool TelemetryData::isWarning(const QString& name) const
{
    if (!m_parameters.contains(name)) {
        return false;
    }
    
    const TelemetryParameter& param = m_parameters[name];
    if (!param.value.canConvert<double>()) {
        return false;
    }
    
    double v = param.value.toDouble();
    
    bool warning = false;
    if (param.warningLow.isValid() && v < param.warningLow.toDouble()) {
        warning = true;
    }
    if (param.warningHigh.isValid() && v > param.warningHigh.toDouble()) {
        warning = true;
    }
    
    // Not warning if critical
    if (isCritical(name)) {
        return false;
    }
    
    return warning;
}

bool TelemetryData::isCritical(const QString& name) const
{
    if (!m_parameters.contains(name)) {
        return false;
    }
    
    const TelemetryParameter& param = m_parameters[name];
    if (!param.value.canConvert<double>()) {
        return false;
    }
    
    double v = param.value.toDouble();
    
    if (param.criticalLow.isValid() && v < param.criticalLow.toDouble()) {
        return true;
    }
    if (param.criticalHigh.isValid() && v > param.criticalHigh.toDouble()) {
        return true;
    }
    
    return false;
}

QVariantMap TelemetryData::getData() const
{
    QVariantMap data;
    for (auto it = m_parameters.begin(); it != m_parameters.end(); ++it) {
        data[it.key()] = it.value().value;
    }
    return data;
}

QVariantMap TelemetryData::getMetadata() const
{
    QVariantMap metadata;
    for (auto it = m_parameters.begin(); it != m_parameters.end(); ++it) {
        metadata[it.key()] = it.value().toVariantMap();
    }
    return metadata;
}

QDateTime TelemetryData::getLastUpdate() const
{
    return m_lastUpdate;
}

void TelemetryData::validate()
{
    for (auto it = m_parameters.begin(); it != m_parameters.end(); ++it) {
        TelemetryParameter& param = it.value();
        bool wasValid = param.isValid;
        
        // Check if value is within physical limits
        if (param.value.canConvert<double>()) {
            double v = param.value.toDouble();
            param.isValid = true;
            
            if (param.minValue.isValid() && v < param.minValue.toDouble()) {
                param.isValid = false;
            }
            if (param.maxValue.isValid() && v > param.maxValue.toDouble()) {
                param.isValid = false;
            }
        }
        
        if (wasValid != param.isValid) {
            emit validityChanged(param.name, param.isValid);
        }
    }
}

bool TelemetryData::isAllValid() const
{
    for (const auto& param : m_parameters) {
        if (!param.isValid) {
            return false;
        }
    }
    return true;
}

QStringList TelemetryData::getInvalidParameters() const
{
    QStringList invalid;
    for (const auto& param : m_parameters) {
        if (!param.isValid) {
            invalid.append(param.name);
        }
    }
    return invalid;
}

} // namespace RadarRMP
