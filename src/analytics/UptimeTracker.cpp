#include "analytics/UptimeTracker.h"

namespace RadarRMP {

UptimeTracker::UptimeTracker(QObject* parent)
    : QObject(parent)
    , m_snapshotIntervalMs(60000)
    , m_lastSnapshotTime(0)
{
    m_trackingStartTime = QDateTime::currentDateTime();
    
    // PERFORMANCE FIX: Disabled tick timer that runs every second
    // Previously, the timer would call tick() which iterates through all records
    // This was causing continuous event loop pressure and unresponsiveness
    // 
    // m_tickTimer = new QTimer(this);
    // connect(m_tickTimer, &QTimer::timeout, this, &UptimeTracker::tick);
    // m_tickTimer->start(1000);  // DISABLED - was ticking every second
}

void UptimeTracker::registerSubsystem(const QString& subsystemId)
{
    if (m_records.contains(subsystemId)) {
        return;
    }
    
    UptimeRecord record;
    record.subsystemId = subsystemId;
    record.startTime = QDateTime::currentDateTime();
    record.totalUptimeMs = 0;
    record.totalDowntimeMs = 0;
    record.currentState = HealthState::UNKNOWN;
    record.lastStateChange = record.startTime;
    record.stateTransitions = 0;
    
    m_records[subsystemId] = record;
}

void UptimeTracker::unregisterSubsystem(const QString& subsystemId)
{
    m_records.remove(subsystemId);
}

void UptimeTracker::updateState(const QString& subsystemId, HealthState state)
{
    if (!m_records.contains(subsystemId)) {
        registerSubsystem(subsystemId);
    }
    
    UptimeRecord& record = m_records[subsystemId];
    
    if (record.currentState != state) {
        QString oldState = healthStateToString(record.currentState);
        QString newState = healthStateToString(state);
        
        // Calculate duration in previous state
        qint64 durationMs = record.lastStateChange.msecsTo(QDateTime::currentDateTime());
        
        if (record.currentState == HealthState::OK || 
            record.currentState == HealthState::DEGRADED) {
            record.totalUptimeMs += durationMs;
        } else if (record.currentState == HealthState::FAIL) {
            record.totalDowntimeMs += durationMs;
            emit outageEnded(subsystemId, durationMs);
        }
        
        record.currentState = state;
        record.lastStateChange = QDateTime::currentDateTime();
        record.stateTransitions++;
        
        if (state == HealthState::FAIL) {
            emit outageStarted(subsystemId);
        }
        
        emit stateChanged(subsystemId, oldState, newState);
        emit uptimeUpdated();
    }
}

void UptimeTracker::recordOutage(const QString& subsystemId, qint64 durationMs)
{
    if (!m_records.contains(subsystemId)) {
        return;
    }
    
    m_records[subsystemId].totalDowntimeMs += durationMs;
    emit uptimeUpdated();
}

UptimeTracker::UptimeRecord UptimeTracker::getUptimeRecord(const QString& subsystemId) const
{
    return m_records.value(subsystemId);
}

double UptimeTracker::getSubsystemUptime(const QString& subsystemId) const
{
    if (!m_records.contains(subsystemId)) {
        return 0;
    }
    
    return m_records[subsystemId].getUptimeHours();
}

double UptimeTracker::getSubsystemAvailability(const QString& subsystemId) const
{
    if (!m_records.contains(subsystemId)) {
        return 100.0;
    }
    
    return m_records[subsystemId].getAvailability();
}

qint64 UptimeTracker::getSubsystemDowntimeMs(const QString& subsystemId) const
{
    if (!m_records.contains(subsystemId)) {
        return 0;
    }
    
    return m_records[subsystemId].totalDowntimeMs;
}

int UptimeTracker::getStateTransitions(const QString& subsystemId) const
{
    if (!m_records.contains(subsystemId)) {
        return 0;
    }
    
    return m_records[subsystemId].stateTransitions;
}

double UptimeTracker::getSystemUptime() const
{
    qint64 totalUptime = 0;
    int count = 0;
    
    for (const UptimeRecord& record : m_records) {
        totalUptime += record.totalUptimeMs;
        count++;
    }
    
    return count > 0 ? (totalUptime / count) / 3600000.0 : 0;
}

double UptimeTracker::getSystemAvailability() const
{
    qint64 totalUp = 0;
    qint64 totalDown = 0;
    
    for (const UptimeRecord& record : m_records) {
        totalUp += record.totalUptimeMs;
        totalDown += record.totalDowntimeMs;
    }
    
    qint64 total = totalUp + totalDown;
    return total > 0 ? (double)totalUp / total * 100.0 : 100.0;
}

QVariantMap UptimeTracker::getSystemUptimeSummary() const
{
    QVariantMap summary;
    
    summary["systemUptime"] = getSystemUptime();
    summary["systemAvailability"] = getSystemAvailability();
    summary["trackingStartTime"] = m_trackingStartTime;
    summary["subsystemCount"] = m_records.size();
    
    QVariantMap subsystemData;
    for (auto it = m_records.begin(); it != m_records.end(); ++it) {
        QVariantMap data;
        data["uptime"] = it.value().getUptimeHours();
        data["availability"] = it.value().getAvailability();
        data["stateTransitions"] = it.value().stateTransitions;
        data["currentState"] = healthStateToString(it.value().currentState);
        subsystemData[it.key()] = data;
    }
    summary["subsystems"] = subsystemData;
    
    return summary;
}

QVariantList UptimeTracker::getUptimeHistory(const QString& subsystemId, int hours) const
{
    QVariantList history;
    
    // Get snapshots for this subsystem within the time range
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-hours * 3600);
    
    for (const HistorySnapshot& snapshot : m_history) {
        if (snapshot.timestamp >= cutoff && snapshot.subsystemAvailability.contains(subsystemId)) {
            QVariantMap entry;
            entry["timestamp"] = snapshot.timestamp;
            entry["availability"] = snapshot.subsystemAvailability[subsystemId];
            history.append(entry);
        }
    }
    
    return history;
}

QVariantList UptimeTracker::getAvailabilityHistory(int hours) const
{
    QVariantList history;
    
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-hours * 3600);
    
    for (const HistorySnapshot& snapshot : m_history) {
        if (snapshot.timestamp >= cutoff) {
            QVariantMap entry;
            entry["timestamp"] = snapshot.timestamp;
            entry["availability"] = snapshot.systemAvailability;
            history.append(entry);
        }
    }
    
    return history;
}

QVariantMap UptimeTracker::generateUptimeReport() const
{
    return getSystemUptimeSummary();
}

QString UptimeTracker::exportUptimeReportCsv() const
{
    QString csv;
    csv += "Subsystem,Uptime (hours),Downtime (hours),Availability (%),State Transitions\n";
    
    for (auto it = m_records.begin(); it != m_records.end(); ++it) {
        const UptimeRecord& record = it.value();
        csv += QString("%1,%2,%3,%4,%5\n")
               .arg(it.key())
               .arg(record.totalUptimeMs / 3600000.0, 0, 'f', 2)
               .arg(record.totalDowntimeMs / 3600000.0, 0, 'f', 2)
               .arg(record.getAvailability(), 0, 'f', 2)
               .arg(record.stateTransitions);
    }
    
    return csv;
}

void UptimeTracker::tick()
{
    updateRunningTotals();
    
    // Take periodic snapshots
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastSnapshotTime >= m_snapshotIntervalMs) {
        HistorySnapshot snapshot;
        snapshot.timestamp = QDateTime::currentDateTime();
        snapshot.systemAvailability = getSystemAvailability();
        
        for (auto it = m_records.begin(); it != m_records.end(); ++it) {
            snapshot.subsystemAvailability[it.key()] = it.value().getAvailability();
        }
        
        m_history.append(snapshot);
        m_lastSnapshotTime = now;
        
        // Prune old history (keep 24 hours)
        QDateTime cutoff = QDateTime::currentDateTime().addSecs(-24 * 3600);
        while (!m_history.isEmpty() && m_history.first().timestamp < cutoff) {
            m_history.removeFirst();
        }
    }
}

void UptimeTracker::reset()
{
    for (auto& record : m_records) {
        record.totalUptimeMs = 0;
        record.totalDowntimeMs = 0;
        record.startTime = QDateTime::currentDateTime();
        record.lastStateChange = record.startTime;
        record.stateTransitions = 0;
    }
    
    m_history.clear();
    m_trackingStartTime = QDateTime::currentDateTime();
    m_lastSnapshotTime = 0;
    
    emit uptimeUpdated();
}

void UptimeTracker::resetSubsystem(const QString& subsystemId)
{
    if (!m_records.contains(subsystemId)) {
        return;
    }
    
    UptimeRecord& record = m_records[subsystemId];
    record.totalUptimeMs = 0;
    record.totalDowntimeMs = 0;
    record.startTime = QDateTime::currentDateTime();
    record.lastStateChange = record.startTime;
    record.stateTransitions = 0;
    
    emit uptimeUpdated();
}

void UptimeTracker::updateRunningTotals()
{
    QDateTime now = QDateTime::currentDateTime();
    
    for (auto& record : m_records) {
        qint64 durationMs = record.lastStateChange.msecsTo(now);
        
        // Add time based on current state
        if (record.currentState == HealthState::OK || 
            record.currentState == HealthState::DEGRADED) {
            record.totalUptimeMs += 1000;  // Add 1 second
        } else if (record.currentState == HealthState::FAIL) {
            record.totalDowntimeMs += 1000;
        }
    }
}

} // namespace RadarRMP
