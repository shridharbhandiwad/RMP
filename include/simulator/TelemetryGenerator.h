#ifndef TELEMETRYGENERATOR_H
#define TELEMETRYGENERATOR_H

#include <QObject>
#include <QMap>
#include <random>
#include "core/HealthStatus.h"

namespace RadarRMP {

/**
 * @brief Telemetry data generator for simulation
 * 
 * Generates realistic telemetry values with configurable:
 * - Nominal values
 * - Variance / noise
 * - Trends (drift)
 * - Anomalies
 */
class TelemetryGenerator : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Parameter generation configuration
     */
    struct ParameterConfig {
        QString name;
        double nominal;          // Nominal value
        double variance;         // Standard deviation
        double minValue;         // Physical minimum
        double maxValue;         // Physical maximum
        double trendRate;        // Change per second (for drift simulation)
        double noiseFrequency;   // Noise variation frequency
        bool bounded;            // Clamp to min/max
        
        ParameterConfig() 
            : nominal(0), variance(0), minValue(0), maxValue(100),
              trendRate(0), noiseFrequency(1.0), bounded(true) {}
              
        ParameterConfig(const QString& n, double nom, double var, 
                       double min = 0, double max = 100)
            : name(n), nominal(nom), variance(var), minValue(min),
              maxValue(max), trendRate(0), noiseFrequency(1.0), bounded(true) {}
    };
    
    explicit TelemetryGenerator(QObject* parent = nullptr);
    ~TelemetryGenerator() override = default;
    
    // Configuration
    void addParameter(const ParameterConfig& config);
    void removeParameter(const QString& name);
    void setParameterConfig(const QString& name, const ParameterConfig& config);
    ParameterConfig getParameterConfig(const QString& name) const;
    
    // Generation
    double generateValue(const QString& name);
    double generateValue(const ParameterConfig& config);
    QVariantMap generateAll();
    
    // State manipulation
    void setTrend(const QString& name, double trendRate);
    void setVariance(const QString& name, double variance);
    void setNominal(const QString& name, double nominal);
    void applyOffset(const QString& name, double offset);
    void reset(const QString& name);
    void resetAll();
    
    // Anomaly injection
    void injectSpike(const QString& name, double magnitude);
    void injectDrift(const QString& name, double driftRate, int durationMs);
    void injectNoise(const QString& name, double noiseFactor);
    void clearAnomalies(const QString& name);
    void clearAllAnomalies();
    
    // Time advancement
    void advanceTime(int milliseconds);
    void setTime(qint64 simulationTimeMs);
    
signals:
    void valueGenerated(const QString& name, double value);
    void anomalyTriggered(const QString& name, const QString& type);
    
private:
    double applyTrend(const ParameterConfig& config, double value);
    double applyNoise(const ParameterConfig& config, double value);
    double applyAnomalies(const QString& name, double value);
    double clampValue(const ParameterConfig& config, double value);
    
    QMap<QString, ParameterConfig> m_parameters;
    QMap<QString, double> m_currentValues;
    QMap<QString, double> m_accumulatedTrend;
    int m_windowSize;
    
    // Anomaly state
    struct Anomaly {
        QString type;
        double magnitude;
        qint64 startTime;
        int durationMs;
    };
    QMap<QString, QList<Anomaly>> m_activeAnomalies;
    
    // Time tracking
    qint64 m_simulationTime;
    
    // Random generation
    std::mt19937 m_rng;
    std::normal_distribution<double> m_normalDist;
};

} // namespace RadarRMP

#endif // TELEMETRYGENERATOR_H
