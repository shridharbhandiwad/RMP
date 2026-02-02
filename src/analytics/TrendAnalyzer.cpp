#include "analytics/TrendAnalyzer.h"
#include <cmath>
#include <algorithm>

namespace RadarRMP {

TrendAnalyzer::TrendAnalyzer(QObject* parent)
    : QObject(parent)
    , m_windowSize(100)
    , m_anomalyThreshold(3.0)  // 3 standard deviations
    , m_trendThreshold(0.001)
    , m_maxDataPoints(10000)
{
}

void TrendAnalyzer::addDataPoint(const QString& subsystemId, const QString& parameter, 
                                  double value, const QDateTime& timestamp)
{
    DataPoint point;
    point.value = value;
    point.timestampMs = timestamp.toMSecsSinceEpoch();
    
    auto& data = m_data[subsystemId][parameter];
    data.push_back(point);
    
    // Limit size
    while (data.size() > static_cast<size_t>(m_maxDataPoints)) {
        data.pop_front();
    }
    
    // Check for anomalies
    if (isAnomaly(subsystemId, parameter, value)) {
        auto& points = m_data[subsystemId][parameter];
        double mean = computeMean(points);
        emit anomalyDetected(subsystemId, parameter, value, mean);
    }
}

void TrendAnalyzer::addDataPoints(const QString& subsystemId, const QVariantMap& values)
{
    QDateTime now = QDateTime::currentDateTime();
    
    for (auto it = values.begin(); it != values.end(); ++it) {
        if (it.value().canConvert<double>()) {
            addDataPoint(subsystemId, it.key(), it.value().toDouble(), now);
        }
    }
}

TrendAnalyzer::TrendResult TrendAnalyzer::analyzeTrend(const QString& subsystemId, 
                                                        const QString& parameter) const
{
    TrendResult result;
    
    if (!m_data.contains(subsystemId) || !m_data[subsystemId].contains(parameter)) {
        return result;
    }
    
    const auto& data = m_data[subsystemId][parameter];
    
    if (data.size() < 3) {
        return result;
    }
    
    // Get recent window
    std::deque<DataPoint> window;
    size_t windowStart = data.size() > static_cast<size_t>(m_windowSize) ? 
                        data.size() - m_windowSize : 0;
    for (size_t i = windowStart; i < data.size(); ++i) {
        window.push_back(data[i]);
    }
    
    // Compute linear regression
    double slope, intercept, rSquared;
    computeLinearRegression(window, slope, intercept, rSquared);
    
    result.slope = slope;
    result.rSquared = rSquared;
    result.currentValue = data.back().value;
    
    // Determine trend direction
    if (std::abs(slope) < m_trendThreshold) {
        result.direction = TrendDirection::Stable;
    } else if (slope > 0) {
        result.direction = TrendDirection::Increasing;
    } else {
        result.direction = TrendDirection::Decreasing;
    }
    
    // Check volatility
    double mean = computeMean(window);
    double stdDev = computeStdDev(window, mean);
    double cv = stdDev / std::abs(mean);  // Coefficient of variation
    
    if (cv > 0.2) {  // High volatility
        result.direction = TrendDirection::Volatile;
    }
    
    // Predict future value (10 seconds ahead)
    double futureTime = data.back().timestampMs + 10000;
    result.predictedValue = slope * futureTime + intercept;
    
    // Compute anomaly score
    result.anomalyScore = std::abs(computeZScore(result.currentValue, mean, stdDev)) / 5.0;
    result.anomalyScore = qBound(0.0, result.anomalyScore, 1.0);
    
    return result;
}

QVariantMap TrendAnalyzer::analyzeTrends(const QString& subsystemId) const
{
    QVariantMap trends;
    
    if (!m_data.contains(subsystemId)) {
        return trends;
    }
    
    for (auto it = m_data[subsystemId].begin(); it != m_data[subsystemId].end(); ++it) {
        TrendResult result = analyzeTrend(subsystemId, it.key());
        
        QVariantMap trendData;
        trendData["direction"] = static_cast<int>(result.direction);
        trendData["slope"] = result.slope;
        trendData["rSquared"] = result.rSquared;
        trendData["currentValue"] = result.currentValue;
        trendData["predictedValue"] = result.predictedValue;
        trendData["anomalyScore"] = result.anomalyScore;
        
        trends[it.key()] = trendData;
    }
    
    return trends;
}

double TrendAnalyzer::predictValue(const QString& subsystemId, const QString& parameter, 
                                    int secondsAhead) const
{
    if (!m_data.contains(subsystemId) || !m_data[subsystemId].contains(parameter)) {
        return 0.0;
    }
    
    const auto& data = m_data[subsystemId][parameter];
    
    if (data.size() < 3) {
        return data.empty() ? 0.0 : data.back().value;
    }
    
    std::deque<DataPoint> window;
    size_t windowStart = data.size() > static_cast<size_t>(m_windowSize) ? 
                        data.size() - m_windowSize : 0;
    for (size_t i = windowStart; i < data.size(); ++i) {
        window.push_back(data[i]);
    }
    
    double slope, intercept, rSquared;
    computeLinearRegression(window, slope, intercept, rSquared);
    
    double futureTime = data.back().timestampMs + secondsAhead * 1000;
    return slope * futureTime + intercept;
}

QDateTime TrendAnalyzer::predictThresholdCrossing(const QString& subsystemId, 
                                                   const QString& parameter,
                                                   double threshold) const
{
    if (!m_data.contains(subsystemId) || !m_data[subsystemId].contains(parameter)) {
        return QDateTime();
    }
    
    const auto& data = m_data[subsystemId][parameter];
    
    if (data.size() < 3) {
        return QDateTime();
    }
    
    std::deque<DataPoint> window;
    size_t windowStart = data.size() > static_cast<size_t>(m_windowSize) ? 
                        data.size() - m_windowSize : 0;
    for (size_t i = windowStart; i < data.size(); ++i) {
        window.push_back(data[i]);
    }
    
    double slope, intercept, rSquared;
    computeLinearRegression(window, slope, intercept, rSquared);
    
    if (std::abs(slope) < 1e-10) {
        return QDateTime();  // No crossing if no slope
    }
    
    // Calculate time when threshold is crossed
    // threshold = slope * time + intercept
    double crossingTime = (threshold - intercept) / slope;
    
    // Only return if in the future
    if (crossingTime > data.back().timestampMs) {
        return QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(crossingTime));
    }
    
    return QDateTime();
}

bool TrendAnalyzer::isAnomaly(const QString& subsystemId, const QString& parameter, 
                               double value) const
{
    if (!m_data.contains(subsystemId) || !m_data[subsystemId].contains(parameter)) {
        return false;
    }
    
    const auto& data = m_data[subsystemId][parameter];
    
    if (data.size() < 10) {
        return false;  // Need enough data for statistics
    }
    
    double mean = computeMean(data);
    double stdDev = computeStdDev(data, mean);
    
    double zScore = computeZScore(value, mean, stdDev);
    
    return std::abs(zScore) > m_anomalyThreshold;
}

QVariantList TrendAnalyzer::detectAnomalies(const QString& subsystemId) const
{
    QVariantList anomalies;
    
    if (!m_data.contains(subsystemId)) {
        return anomalies;
    }
    
    for (auto it = m_data[subsystemId].begin(); it != m_data[subsystemId].end(); ++it) {
        const auto& data = it.value();
        
        if (data.size() < 10) {
            continue;
        }
        
        double mean = computeMean(data);
        double stdDev = computeStdDev(data, mean);
        
        // Check last value
        double zScore = computeZScore(data.back().value, mean, stdDev);
        
        if (std::abs(zScore) > m_anomalyThreshold) {
            QVariantMap anomaly;
            anomaly["parameter"] = it.key();
            anomaly["value"] = data.back().value;
            anomaly["expected"] = mean;
            anomaly["zScore"] = zScore;
            anomalies.append(anomaly);
        }
    }
    
    return anomalies;
}

void TrendAnalyzer::setWindowSize(int samples)
{
    m_windowSize = samples;
}

int TrendAnalyzer::getWindowSize() const
{
    return m_windowSize;
}

void TrendAnalyzer::setAnomalyThreshold(double threshold)
{
    m_anomalyThreshold = threshold;
}

double TrendAnalyzer::getAnomalyThreshold() const
{
    return m_anomalyThreshold;
}

void TrendAnalyzer::setTrendThreshold(double threshold)
{
    m_trendThreshold = threshold;
}

QVariantList TrendAnalyzer::getDataPoints(const QString& subsystemId, const QString& parameter,
                                           int maxPoints) const
{
    QVariantList points;
    
    if (!m_data.contains(subsystemId) || !m_data[subsystemId].contains(parameter)) {
        return points;
    }
    
    const auto& data = m_data[subsystemId][parameter];
    
    size_t start = data.size() > static_cast<size_t>(maxPoints) ? 
                   data.size() - maxPoints : 0;
    
    for (size_t i = start; i < data.size(); ++i) {
        QVariantMap point;
        point["timestamp"] = QDateTime::fromMSecsSinceEpoch(data[i].timestampMs);
        point["value"] = data[i].value;
        points.append(point);
    }
    
    return points;
}

QVariantList TrendAnalyzer::getTrendLine(const QString& subsystemId, const QString& parameter,
                                          int points) const
{
    QVariantList trendLine;
    
    if (!m_data.contains(subsystemId) || !m_data[subsystemId].contains(parameter)) {
        return trendLine;
    }
    
    const auto& data = m_data[subsystemId][parameter];
    
    if (data.size() < 3) {
        return trendLine;
    }
    
    std::deque<DataPoint> window;
    size_t windowStart = data.size() > static_cast<size_t>(m_windowSize) ? 
                        data.size() - m_windowSize : 0;
    for (size_t i = windowStart; i < data.size(); ++i) {
        window.push_back(data[i]);
    }
    
    double slope, intercept, rSquared;
    computeLinearRegression(window, slope, intercept, rSquared);
    
    // Generate trend line points
    qint64 startTime = data.front().timestampMs;
    qint64 endTime = data.back().timestampMs;
    qint64 step = (endTime - startTime) / (points - 1);
    
    for (int i = 0; i < points; ++i) {
        qint64 t = startTime + i * step;
        double v = slope * t + intercept;
        
        QVariantMap point;
        point["timestamp"] = QDateTime::fromMSecsSinceEpoch(t);
        point["value"] = v;
        trendLine.append(point);
    }
    
    return trendLine;
}

void TrendAnalyzer::clearData(const QString& subsystemId)
{
    m_data.remove(subsystemId);
}

void TrendAnalyzer::clearAllData()
{
    m_data.clear();
}

void TrendAnalyzer::pruneOldData(int maxAgeHours)
{
    qint64 cutoff = QDateTime::currentMSecsSinceEpoch() - maxAgeHours * 3600000;
    
    for (auto& subsystemData : m_data) {
        for (auto& paramData : subsystemData) {
            while (!paramData.empty() && paramData.front().timestampMs < cutoff) {
                paramData.pop_front();
            }
        }
    }
}

void TrendAnalyzer::computeLinearRegression(const std::deque<DataPoint>& data,
                                             double& slope, double& intercept, 
                                             double& rSquared) const
{
    if (data.size() < 2) {
        slope = 0;
        intercept = 0;
        rSquared = 0;
        return;
    }
    
    double n = data.size();
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0, sumY2 = 0;
    
    for (const auto& point : data) {
        double x = point.timestampMs;
        double y = point.value;
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
        sumY2 += y * y;
    }
    
    double denom = n * sumX2 - sumX * sumX;
    
    if (std::abs(denom) < 1e-10) {
        slope = 0;
        intercept = sumY / n;
        rSquared = 0;
        return;
    }
    
    slope = (n * sumXY - sumX * sumY) / denom;
    intercept = (sumY - slope * sumX) / n;
    
    // Calculate R-squared
    double meanY = sumY / n;
    double ssTot = 0, ssRes = 0;
    
    for (const auto& point : data) {
        double predicted = slope * point.timestampMs + intercept;
        ssRes += (point.value - predicted) * (point.value - predicted);
        ssTot += (point.value - meanY) * (point.value - meanY);
    }
    
    rSquared = ssTot > 0 ? 1 - (ssRes / ssTot) : 0;
}

double TrendAnalyzer::computeMean(const std::deque<DataPoint>& data) const
{
    if (data.empty()) {
        return 0;
    }
    
    double sum = 0;
    for (const auto& point : data) {
        sum += point.value;
    }
    return sum / data.size();
}

double TrendAnalyzer::computeStdDev(const std::deque<DataPoint>& data, double mean) const
{
    if (data.size() < 2) {
        return 0;
    }
    
    double sumSq = 0;
    for (const auto& point : data) {
        double diff = point.value - mean;
        sumSq += diff * diff;
    }
    
    return std::sqrt(sumSq / (data.size() - 1));
}

double TrendAnalyzer::computeZScore(double value, double mean, double stdDev) const
{
    if (stdDev < 1e-10) {
        return 0;
    }
    return (value - mean) / stdDev;
}

} // namespace RadarRMP
