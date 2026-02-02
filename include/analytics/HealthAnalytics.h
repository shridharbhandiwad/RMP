#ifndef HEALTHANALYTICS_H
#define HEALTHANALYTICS_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QVariantList>
#include <QTimer>
#include "core/HealthStatus.h"

namespace RadarRMP {

class SubsystemManager;

/**
 * @brief System-wide health analytics
 * 
 * Provides comprehensive analytics including:
 * - Health summaries
 * - Fault statistics
 * - MTBF calculations
 * - Trend analysis
 * - Predictive insights
 */
class HealthAnalytics : public QObject {
    Q_OBJECT
    Q_PROPERTY(double systemAvailability READ getSystemAvailability NOTIFY analyticsUpdated)
    Q_PROPERTY(double averageHealthScore READ getAverageHealthScore NOTIFY analyticsUpdated)
    Q_PROPERTY(int totalFaults READ getTotalFaults NOTIFY analyticsUpdated)
    Q_PROPERTY(QVariantMap healthSummary READ getHealthSummary NOTIFY analyticsUpdated)
    
public:
    explicit HealthAnalytics(SubsystemManager* manager, QObject* parent = nullptr);
    ~HealthAnalytics() override = default;
    
    // System-wide metrics
    double getSystemAvailability() const;      // Percentage
    double getAverageHealthScore() const;      // 0-100
    int getTotalFaults() const;
    QVariantMap getHealthSummary() const;
    
    // Per-subsystem analytics
    Q_INVOKABLE QVariantMap getSubsystemAnalytics(const QString& subsystemId) const;
    Q_INVOKABLE double getSubsystemUptime(const QString& subsystemId) const;
    Q_INVOKABLE double getSubsystemMTBF(const QString& subsystemId) const;
    Q_INVOKABLE double getSubsystemMTTR(const QString& subsystemId) const;
    Q_INVOKABLE int getSubsystemFaultCount(const QString& subsystemId) const;
    
    // Historical data
    Q_INVOKABLE QVariantList getHealthHistory(const QString& subsystemId, int hours = 24) const;
    Q_INVOKABLE QVariantList getFaultHistory(const QString& subsystemId, int maxCount = 100) const;
    Q_INVOKABLE QVariantList getTelemetryHistory(const QString& subsystemId, 
                                                  const QString& parameter, 
                                                  int hours = 24) const;
    
    // Aggregated metrics
    Q_INVOKABLE QVariantMap getFaultStatistics() const;
    Q_INVOKABLE QVariantList getTopFaults(int count = 10) const;
    Q_INVOKABLE QVariantMap getSubsystemRanking() const;
    
    // Trend data for charts
    Q_INVOKABLE QVariantList getHealthScoreTrend(int hours = 24) const;
    Q_INVOKABLE QVariantList getTemperatureTrend(int hours = 24) const;
    Q_INVOKABLE QVariantList getFaultRateTrend(int hours = 24) const;
    
    // Reports
    Q_INVOKABLE QVariantMap generateReport(const QDateTime& startTime, 
                                           const QDateTime& endTime) const;
    Q_INVOKABLE QString exportReportCsv(const QDateTime& startTime,
                                        const QDateTime& endTime) const;
    
public slots:
    void updateAnalytics();
    void recordHealthSnapshot();
    void onSubsystemHealthChanged(const QString& subsystemId);
    void onFaultOccurred(const QString& subsystemId, const QString& faultCode);
    void onFaultCleared(const QString& subsystemId, const QString& faultCode);
    
signals:
    void analyticsUpdated();
    void alertGenerated(const QString& subsystemId, const QString& alertType, 
                       const QString& message);
    
private:
    void initializeTracking();
    void computeMetrics();
    void checkAlertConditions();
    
    SubsystemManager* m_manager;
    
    // Health history storage
    struct HealthRecord {
        QDateTime timestamp;
        HealthState state;
        double healthScore;
        QVariantMap telemetry;
    };
    QMap<QString, QList<HealthRecord>> m_healthHistory;
    
    // Fault tracking
    struct FaultRecord {
        QString faultCode;
        QDateTime startTime;
        QDateTime endTime;
        int durationMs;
        bool resolved;
    };
    QMap<QString, QList<FaultRecord>> m_faultHistory;
    
    // Uptime tracking
    QMap<QString, QDateTime> m_subsystemStartTimes;
    QMap<QString, qint64> m_uptimeMs;
    QMap<QString, qint64> m_downtimeMs;
    
    // Computed metrics
    double m_systemAvailability;
    double m_averageHealthScore;
    int m_totalFaults;
    
    // Configuration
    int m_historyRetentionHours;
    int m_snapshotIntervalMs;
    QTimer* m_snapshotTimer;
};

} // namespace RadarRMP

#endif // HEALTHANALYTICS_H
