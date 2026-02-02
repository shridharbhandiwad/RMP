#ifndef UPTIMETRACKER_H
#define UPTIMETRACKER_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QTimer>
#include "core/HealthStatus.h"

namespace RadarRMP {

/**
 * @brief Tracks uptime and availability metrics for subsystems
 */
class UptimeTracker : public QObject {
    Q_OBJECT
    Q_PROPERTY(double systemUptime READ getSystemUptime NOTIFY uptimeUpdated)
    Q_PROPERTY(double systemAvailability READ getSystemAvailability NOTIFY uptimeUpdated)
    
public:
    /**
     * @brief Uptime record for a subsystem
     */
    struct UptimeRecord {
        QString subsystemId;
        QDateTime startTime;
        qint64 totalUptimeMs;
        qint64 totalDowntimeMs;
        HealthState currentState;
        QDateTime lastStateChange;
        int stateTransitions;
        
        double getAvailability() const {
            qint64 total = totalUptimeMs + totalDowntimeMs;
            return total > 0 ? (double)totalUptimeMs / total * 100.0 : 100.0;
        }
        
        double getUptimeHours() const {
            return totalUptimeMs / 3600000.0;
        }
    };
    
    explicit UptimeTracker(QObject* parent = nullptr);
    ~UptimeTracker() override = default;
    
    // Registration
    void registerSubsystem(const QString& subsystemId);
    void unregisterSubsystem(const QString& subsystemId);
    
    // State updates
    void updateState(const QString& subsystemId, HealthState state);
    void recordOutage(const QString& subsystemId, qint64 durationMs);
    
    // Queries
    UptimeRecord getUptimeRecord(const QString& subsystemId) const;
    double getSubsystemUptime(const QString& subsystemId) const;
    double getSubsystemAvailability(const QString& subsystemId) const;
    qint64 getSubsystemDowntimeMs(const QString& subsystemId) const;
    int getStateTransitions(const QString& subsystemId) const;
    
    // System-wide metrics
    double getSystemUptime() const;
    double getSystemAvailability() const;
    QVariantMap getSystemUptimeSummary() const;
    
    // Historical data
    QVariantList getUptimeHistory(const QString& subsystemId, int hours = 24) const;
    QVariantList getAvailabilityHistory(int hours = 24) const;
    
    // Reporting
    QVariantMap generateUptimeReport() const;
    QString exportUptimeReportCsv() const;
    
public slots:
    void tick();  // Called periodically to update running totals
    void reset();
    void resetSubsystem(const QString& subsystemId);
    
signals:
    void uptimeUpdated();
    void stateChanged(const QString& subsystemId, const QString& oldState, 
                     const QString& newState);
    void outageStarted(const QString& subsystemId);
    void outageEnded(const QString& subsystemId, qint64 durationMs);
    
private:
    void updateRunningTotals();
    
    QMap<QString, UptimeRecord> m_records;
    QTimer* m_tickTimer;
    QDateTime m_trackingStartTime;
    
    // Historical snapshots
    struct HistorySnapshot {
        QDateTime timestamp;
        double systemAvailability;
        QMap<QString, double> subsystemAvailability;
    };
    QList<HistorySnapshot> m_history;
    int m_snapshotIntervalMs;
    qint64 m_lastSnapshotTime;
};

} // namespace RadarRMP

#endif // UPTIMETRACKER_H
