#include "simulator/TelemetryGenerator.h"
#include <cmath>

namespace RadarRMP {

TelemetryGenerator::TelemetryGenerator(QObject* parent)
    : QObject(parent)
    , m_windowSize(100)
    , m_simulationTime(0)
    , m_rng(std::random_device{}())
    , m_normalDist(0.0, 1.0)
{
}

void TelemetryGenerator::addParameter(const ParameterConfig& config)
{
    m_parameters[config.name] = config;
    m_currentValues[config.name] = config.nominal;
    m_accumulatedTrend[config.name] = 0.0;
}

void TelemetryGenerator::removeParameter(const QString& name)
{
    m_parameters.remove(name);
    m_currentValues.remove(name);
    m_accumulatedTrend.remove(name);
    m_activeAnomalies.remove(name);
}

void TelemetryGenerator::setParameterConfig(const QString& name, const ParameterConfig& config)
{
    m_parameters[name] = config;
}

TelemetryGenerator::ParameterConfig TelemetryGenerator::getParameterConfig(const QString& name) const
{
    return m_parameters.value(name);
}

double TelemetryGenerator::generateValue(const QString& name)
{
    if (!m_parameters.contains(name)) {
        return 0.0;
    }
    
    return generateValue(m_parameters[name]);
}

double TelemetryGenerator::generateValue(const ParameterConfig& config)
{
    double value = config.nominal;
    
    // Apply variance/noise
    value = applyNoise(config, value);
    
    // Apply trend
    value = applyTrend(config, value);
    
    // Apply anomalies
    value = applyAnomalies(config.name, value);
    
    // Clamp if bounded
    value = clampValue(config, value);
    
    m_currentValues[config.name] = value;
    emit valueGenerated(config.name, value);
    
    return value;
}

QVariantMap TelemetryGenerator::generateAll()
{
    QVariantMap values;
    
    for (const QString& name : m_parameters.keys()) {
        values[name] = generateValue(name);
    }
    
    return values;
}

void TelemetryGenerator::setTrend(const QString& name, double trendRate)
{
    if (m_parameters.contains(name)) {
        m_parameters[name].trendRate = trendRate;
    }
}

void TelemetryGenerator::setVariance(const QString& name, double variance)
{
    if (m_parameters.contains(name)) {
        m_parameters[name].variance = variance;
    }
}

void TelemetryGenerator::setNominal(const QString& name, double nominal)
{
    if (m_parameters.contains(name)) {
        m_parameters[name].nominal = nominal;
    }
}

void TelemetryGenerator::applyOffset(const QString& name, double offset)
{
    if (m_parameters.contains(name)) {
        m_parameters[name].nominal += offset;
    }
}

void TelemetryGenerator::reset(const QString& name)
{
    if (m_parameters.contains(name)) {
        m_currentValues[name] = m_parameters[name].nominal;
        m_accumulatedTrend[name] = 0.0;
        m_activeAnomalies.remove(name);
    }
}

void TelemetryGenerator::resetAll()
{
    for (const QString& name : m_parameters.keys()) {
        reset(name);
    }
    m_simulationTime = 0;
}

void TelemetryGenerator::injectSpike(const QString& name, double magnitude)
{
    Anomaly a;
    a.type = "spike";
    a.magnitude = magnitude;
    a.startTime = m_simulationTime;
    a.durationMs = 100;  // Short duration spike
    
    m_activeAnomalies[name].append(a);
    emit anomalyTriggered(name, "spike");
}

void TelemetryGenerator::injectDrift(const QString& name, double driftRate, int durationMs)
{
    Anomaly a;
    a.type = "drift";
    a.magnitude = driftRate;
    a.startTime = m_simulationTime;
    a.durationMs = durationMs;
    
    m_activeAnomalies[name].append(a);
    emit anomalyTriggered(name, "drift");
}

void TelemetryGenerator::injectNoise(const QString& name, double noiseFactor)
{
    Anomaly a;
    a.type = "noise";
    a.magnitude = noiseFactor;
    a.startTime = m_simulationTime;
    a.durationMs = -1;  // Permanent until cleared
    
    m_activeAnomalies[name].append(a);
    emit anomalyTriggered(name, "noise");
}

void TelemetryGenerator::clearAnomalies(const QString& name)
{
    m_activeAnomalies.remove(name);
}

void TelemetryGenerator::clearAllAnomalies()
{
    m_activeAnomalies.clear();
}

void TelemetryGenerator::advanceTime(int milliseconds)
{
    m_simulationTime += milliseconds;
    
    // Update accumulated trends
    for (const QString& name : m_parameters.keys()) {
        const ParameterConfig& config = m_parameters[name];
        m_accumulatedTrend[name] += config.trendRate * (milliseconds / 1000.0);
    }
    
    // Remove expired anomalies
    for (auto it = m_activeAnomalies.begin(); it != m_activeAnomalies.end(); ++it) {
        QList<Anomaly>& anomalies = it.value();
        anomalies.erase(
            std::remove_if(anomalies.begin(), anomalies.end(),
                [this](const Anomaly& a) {
                    return a.durationMs > 0 && 
                           (m_simulationTime - a.startTime) > a.durationMs;
                }),
            anomalies.end()
        );
    }
}

void TelemetryGenerator::setTime(qint64 simulationTimeMs)
{
    m_simulationTime = simulationTimeMs;
}

double TelemetryGenerator::applyTrend(const ParameterConfig& config, double value)
{
    return value + m_accumulatedTrend.value(config.name, 0.0);
}

double TelemetryGenerator::applyNoise(const ParameterConfig& config, double value)
{
    if (config.variance > 0) {
        double noise = m_normalDist(m_rng) * config.variance;
        
        // Add some frequency-dependent variation
        double t = m_simulationTime / 1000.0;
        double periodicNoise = std::sin(t * config.noiseFrequency * 2 * M_PI) * config.variance * 0.3;
        
        return value + noise + periodicNoise;
    }
    return value;
}

double TelemetryGenerator::applyAnomalies(const QString& name, double value)
{
    if (!m_activeAnomalies.contains(name)) {
        return value;
    }
    
    double result = value;
    
    for (const Anomaly& a : m_activeAnomalies[name]) {
        if (a.type == "spike") {
            // Apply spike if within duration
            qint64 elapsed = m_simulationTime - a.startTime;
            if (elapsed < a.durationMs) {
                result += a.magnitude;
            }
        } else if (a.type == "drift") {
            // Apply gradual drift
            qint64 elapsed = m_simulationTime - a.startTime;
            if (elapsed < a.durationMs || a.durationMs < 0) {
                result += a.magnitude * (elapsed / 1000.0);
            }
        } else if (a.type == "noise") {
            // Apply additional noise
            result += m_normalDist(m_rng) * a.magnitude;
        }
    }
    
    return result;
}

double TelemetryGenerator::clampValue(const ParameterConfig& config, double value)
{
    if (config.bounded) {
        return qBound(config.minValue, value, config.maxValue);
    }
    return value;
}

} // namespace RadarRMP
