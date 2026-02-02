#include "core/FaultManager.h"

namespace RadarRMP {

FaultManager::FaultManager(QObject* parent)
    : QObject(parent)
{
}

void FaultManager::registerFault(const FaultCode& fault)
{
    QString key = makeFaultKey(fault.subsystemId, fault.code);
    
    if (m_activeFaults.contains(key)) {
        return;  // Fault already registered
    }
    
    m_activeFaults[key] = fault;
    m_faultHistory.append(fault);
    
    // Update subsystem fault tracking
    m_subsystemLastFault[fault.subsystemId] = fault.timestamp;
    m_subsystemFaultCounts[fault.subsystemId]++;
    
    // Trim history if needed
    while (m_faultHistory.size() > MAX_HISTORY_SIZE) {
        m_faultHistory.removeFirst();
    }
    
    emit faultRegistered(fault.subsystemId, fault.code);
    emit faultsChanged();
    
    if (fault.severity == FaultSeverity::CRITICAL || 
        fault.severity == FaultSeverity::FATAL) {
        emit criticalFaultOccurred(fault.subsystemId, fault.code);
    }
}

void FaultManager::clearFault(const QString& faultCode, const QString& subsystemId)
{
    QString key = makeFaultKey(subsystemId, faultCode);
    
    if (m_activeFaults.remove(key) > 0) {
        emit faultCleared(subsystemId, faultCode);
        emit faultsChanged();
    }
}

void FaultManager::clearAllFaults(const QString& subsystemId)
{
    QStringList keysToRemove;
    
    for (auto it = m_activeFaults.begin(); it != m_activeFaults.end(); ++it) {
        if (it.value().subsystemId == subsystemId) {
            keysToRemove.append(it.key());
        }
    }
    
    for (const QString& key : keysToRemove) {
        FaultCode fault = m_activeFaults.take(key);
        emit faultCleared(fault.subsystemId, fault.code);
    }
    
    if (!keysToRemove.isEmpty()) {
        emit faultsChanged();
    }
}

void FaultManager::clearAllFaults()
{
    if (m_activeFaults.isEmpty()) {
        return;
    }
    
    for (const auto& fault : m_activeFaults) {
        emit faultCleared(fault.subsystemId, fault.code);
    }
    
    m_activeFaults.clear();
    emit faultsChanged();
}

QList<FaultCode> FaultManager::getActiveFaults() const
{
    return m_activeFaults.values();
}

QList<FaultCode> FaultManager::getActiveFaults(const QString& subsystemId) const
{
    QList<FaultCode> faults;
    
    for (const auto& fault : m_activeFaults) {
        if (fault.subsystemId == subsystemId) {
            faults.append(fault);
        }
    }
    
    return faults;
}

QList<FaultCode> FaultManager::getFaultHistory(int maxCount) const
{
    QList<FaultCode> history;
    
    int start = qMax(0, m_faultHistory.size() - maxCount);
    for (int i = m_faultHistory.size() - 1; i >= start; --i) {
        history.append(m_faultHistory[i]);
    }
    
    return history;
}

QList<FaultCode> FaultManager::getFaultHistory(const QString& subsystemId, int maxCount) const
{
    QList<FaultCode> history;
    
    for (int i = m_faultHistory.size() - 1; i >= 0 && history.size() < maxCount; --i) {
        if (m_faultHistory[i].subsystemId == subsystemId) {
            history.append(m_faultHistory[i]);
        }
    }
    
    return history;
}

bool FaultManager::hasFault(const QString& faultCode) const
{
    for (const auto& fault : m_activeFaults) {
        if (fault.code == faultCode) {
            return true;
        }
    }
    return false;
}

FaultCode FaultManager::getFault(const QString& faultCode) const
{
    for (const auto& fault : m_activeFaults) {
        if (fault.code == faultCode) {
            return fault;
        }
    }
    return FaultCode();
}

int FaultManager::getTotalActiveFaults() const
{
    return m_activeFaults.size();
}

int FaultManager::getCriticalFaultCount() const
{
    int count = 0;
    for (const auto& fault : m_activeFaults) {
        if (fault.severity == FaultSeverity::CRITICAL || 
            fault.severity == FaultSeverity::FATAL) {
            count++;
        }
    }
    return count;
}

int FaultManager::getFaultCount(FaultSeverity severity) const
{
    int count = 0;
    for (const auto& fault : m_activeFaults) {
        if (fault.severity == severity) {
            count++;
        }
    }
    return count;
}

int FaultManager::getFaultCount(const QString& subsystemId) const
{
    int count = 0;
    for (const auto& fault : m_activeFaults) {
        if (fault.subsystemId == subsystemId) {
            count++;
        }
    }
    return count;
}

QVariantList FaultManager::getActiveFaultsVariant() const
{
    QVariantList list;
    
    for (const auto& fault : m_activeFaults) {
        QVariantMap map;
        map["code"] = fault.code;
        map["description"] = fault.description;
        map["severity"] = faultSeverityToString(fault.severity);
        map["timestamp"] = fault.timestamp;
        map["subsystemId"] = fault.subsystemId;
        map["active"] = fault.active;
        list.append(map);
    }
    
    return list;
}

QVariantList FaultManager::getRecentFaultsVariant(int maxCount) const
{
    QVariantList list;
    auto history = getFaultHistory(maxCount);
    
    for (const auto& fault : history) {
        QVariantMap map;
        map["code"] = fault.code;
        map["description"] = fault.description;
        map["severity"] = faultSeverityToString(fault.severity);
        map["timestamp"] = fault.timestamp;
        map["subsystemId"] = fault.subsystemId;
        map["active"] = fault.active;
        list.append(map);
    }
    
    return list;
}

QVariantMap FaultManager::getFaultStatistics() const
{
    QVariantMap stats;
    
    stats["totalActive"] = getTotalActiveFaults();
    stats["criticalCount"] = getCriticalFaultCount();
    stats["warningCount"] = getFaultCount(FaultSeverity::WARNING);
    stats["infoCount"] = getFaultCount(FaultSeverity::INFO);
    stats["historyCount"] = m_faultHistory.size();
    
    // Per-subsystem counts
    QVariantMap subsystemCounts;
    for (auto it = m_subsystemFaultCounts.begin(); it != m_subsystemFaultCounts.end(); ++it) {
        subsystemCounts[it.key()] = it.value();
    }
    stats["subsystemCounts"] = subsystemCounts;
    
    return stats;
}

double FaultManager::estimateMTBF(const QString& subsystemId) const
{
    int faultCount = m_subsystemFaultCounts.value(subsystemId, 0);
    
    if (faultCount < 2) {
        return -1;  // Not enough data
    }
    
    // Simple MTBF estimation based on fault count and tracking time
    // In a real system, this would use more sophisticated analysis
    QDateTime firstFault;
    QDateTime lastFault;
    
    for (const auto& fault : m_faultHistory) {
        if (fault.subsystemId == subsystemId) {
            if (!firstFault.isValid() || fault.timestamp < firstFault) {
                firstFault = fault.timestamp;
            }
            if (!lastFault.isValid() || fault.timestamp > lastFault) {
                lastFault = fault.timestamp;
            }
        }
    }
    
    if (!firstFault.isValid() || !lastFault.isValid()) {
        return -1;
    }
    
    qint64 totalTimeMs = firstFault.msecsTo(lastFault);
    if (totalTimeMs <= 0) {
        return -1;
    }
    
    // MTBF in hours
    return (totalTimeMs / 3600000.0) / (faultCount - 1);
}

QVariantMap FaultManager::getMTBFReport() const
{
    QVariantMap report;
    
    for (const QString& subsystemId : m_subsystemFaultCounts.keys()) {
        double mtbf = estimateMTBF(subsystemId);
        if (mtbf > 0) {
            report[subsystemId] = mtbf;
        }
    }
    
    return report;
}

void FaultManager::onSubsystemFault(const QString& subsystemId, const FaultCode& fault)
{
    FaultCode f = fault;
    f.subsystemId = subsystemId;
    registerFault(f);
}

void FaultManager::onSubsystemFaultCleared(const QString& subsystemId, const QString& faultCode)
{
    clearFault(faultCode, subsystemId);
}

QString FaultManager::makeFaultKey(const QString& subsystemId, const QString& faultCode) const
{
    return subsystemId + ":" + faultCode;
}

} // namespace RadarRMP
