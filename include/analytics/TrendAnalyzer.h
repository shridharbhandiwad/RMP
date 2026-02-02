#ifndef TRENDANALYZER_H
#define TRENDANALYZER_H

#include <QObject>
#include <QMap>
#include <QVariantList>
#include <QDateTime>
#include <deque>

namespace RadarRMP {

/**
 * @brief Trend analysis for telemetry parameters
 * 
 * Analyzes parameter trends over time to detect:
 * - Gradual degradation
 * - Anomalies
 * - Predictive warnings
 */
class TrendAnalyzer : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Trend direction
     */
    enum class TrendDirection {
        Stable,
        Increasing,
        Decreasing,
        Volatile
    };
    Q_ENUM(TrendDirection)
    
    /**
     * @brief Trend analysis result
     */
    struct TrendResult {
        TrendDirection direction;
        double slope;               // Rate of change
        double rSquared;            // Goodness of fit
        double currentValue;
        double predictedValue;      // Predicted future value
        double anomalyScore;        // 0-1, higher = more anomalous
        QString warningMessage;
        
        TrendResult() 
            : direction(TrendDirection::Stable), slope(0), rSquared(0),
              currentValue(0), predictedValue(0), anomalyScore(0) {}
    };
    
    explicit TrendAnalyzer(QObject* parent = nullptr);
    ~TrendAnalyzer() override = default;
    
    // Data input
    void addDataPoint(const QString& subsystemId, const QString& parameter, 
                     double value, const QDateTime& timestamp = QDateTime::currentDateTime());
    void addDataPoints(const QString& subsystemId, const QVariantMap& values);
    
    // Analysis
    TrendResult analyzeTrend(const QString& subsystemId, const QString& parameter) const;
    QVariantMap analyzeTrends(const QString& subsystemId) const;
    
    // Predictions
    double predictValue(const QString& subsystemId, const QString& parameter, 
                       int secondsAhead) const;
    QDateTime predictThresholdCrossing(const QString& subsystemId, const QString& parameter,
                                       double threshold) const;
    
    // Anomaly detection
    bool isAnomaly(const QString& subsystemId, const QString& parameter, double value) const;
    QVariantList detectAnomalies(const QString& subsystemId) const;
    
    // Configuration
    void setWindowSize(int samples);
    int getWindowSize() const;
    void setAnomalyThreshold(double threshold);
    double getAnomalyThreshold() const;
    void setTrendThreshold(double threshold);
    
    // Data access for charting
    QVariantList getDataPoints(const QString& subsystemId, const QString& parameter,
                              int maxPoints = 100) const;
    QVariantList getTrendLine(const QString& subsystemId, const QString& parameter,
                             int points = 100) const;
    
    // Maintenance
    void clearData(const QString& subsystemId);
    void clearAllData();
    void pruneOldData(int maxAgeHours = 24);
    
signals:
    void trendChanged(const QString& subsystemId, const QString& parameter,
                     const QString& direction);
    void anomalyDetected(const QString& subsystemId, const QString& parameter,
                        double value, double expected);
    void thresholdWarning(const QString& subsystemId, const QString& parameter,
                         const QString& message);
    
private:
    struct DataPoint {
        double value;
        qint64 timestampMs;
    };
    
    // Linear regression
    void computeLinearRegression(const std::deque<DataPoint>& data,
                                double& slope, double& intercept, double& rSquared) const;
    
    // Statistical analysis
    double computeMean(const std::deque<DataPoint>& data) const;
    double computeStdDev(const std::deque<DataPoint>& data, double mean) const;
    double computeZScore(double value, double mean, double stdDev) const;
    
    // Data storage: subsystemId -> parameter -> data points
    QMap<QString, QMap<QString, std::deque<DataPoint>>> m_data;
    
    // Configuration
    int m_windowSize;
    double m_anomalyThreshold;  // Z-score threshold
    double m_trendThreshold;    // Minimum slope to consider trending
    int m_maxDataPoints;
};

} // namespace RadarRMP

#endif // TRENDANALYZER_H
