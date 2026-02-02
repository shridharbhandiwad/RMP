#include "core/SubsystemManager.h"

namespace RadarRMP {

SubsystemManager::SubsystemManager(QObject* parent)
    : QObject(parent)
    , m_systemHealthState(HealthState::UNKNOWN)
    , m_systemHealthScore(100.0)
    , m_updateInterval(1000)
{
    m_faultManager = new FaultManager(this);
    
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &SubsystemManager::updateAll);
}

SubsystemManager::~SubsystemManager()
{
    stopUpdates();
}

void SubsystemManager::registerSubsystem(RadarSubsystem* subsystem)
{
    if (!subsystem || m_subsystems.contains(subsystem->getId())) {
        return;
    }
    
    m_subsystems[subsystem->getId()] = subsystem;
    
    // Take ownership if no parent
    if (!subsystem->parent()) {
        subsystem->setParent(this);
    }
    
    connectSubsystemSignals(subsystem);
    
    emit subsystemsChanged();
    computeSystemHealth();
}

void SubsystemManager::unregisterSubsystem(const QString& id)
{
    if (!m_subsystems.contains(id)) {
        return;
    }
    
    // Remove from active list if present
    m_activeSubsystemIds.removeAll(id);
    
    RadarSubsystem* subsystem = m_subsystems.take(id);
    
    // Clear any active faults
    m_faultManager->clearAllFaults(id);
    
    // Don't delete if it has a different parent
    if (subsystem->parent() == this) {
        subsystem->deleteLater();
    }
    
    emit subsystemsChanged();
    emit activeSubsystemsChanged();
    computeSystemHealth();
}

RadarSubsystem* SubsystemManager::getSubsystem(const QString& id) const
{
    return m_subsystems.value(id, nullptr);
}

QList<RadarSubsystem*> SubsystemManager::getAllSubsystems() const
{
    return m_subsystems.values();
}

QList<RadarSubsystem*> SubsystemManager::getSubsystemsByType(SubsystemType type) const
{
    QList<RadarSubsystem*> result;
    
    for (auto* subsystem : m_subsystems) {
        if (subsystem->getType() == type) {
            result.append(subsystem);
        }
    }
    
    return result;
}

void SubsystemManager::addToCanvas(const QString& subsystemId)
{
    if (!m_subsystems.contains(subsystemId)) {
        return;
    }
    
    if (!m_activeSubsystemIds.contains(subsystemId)) {
        m_activeSubsystemIds.append(subsystemId);
        emit activeSubsystemsChanged();
        computeSystemHealth();
    }
}

void SubsystemManager::removeFromCanvas(const QString& subsystemId)
{
    if (m_activeSubsystemIds.removeAll(subsystemId) > 0) {
        emit activeSubsystemsChanged();
        computeSystemHealth();
    }
}

bool SubsystemManager::isOnCanvas(const QString& subsystemId) const
{
    return m_activeSubsystemIds.contains(subsystemId);
}

QVariantList SubsystemManager::getActiveSubsystemsVariant() const
{
    QVariantList list;
    
    for (const QString& id : m_activeSubsystemIds) {
        if (RadarSubsystem* sub = m_subsystems.value(id)) {
            QVariantMap map;
            map["id"] = sub->getId();
            map["name"] = sub->getName();
            map["type"] = sub->getTypeName();
            map["healthState"] = sub->getHealthStateString();
            map["healthScore"] = sub->getHealthScore();
            map["faultCount"] = sub->getFaultCount();
            map["enabled"] = sub->isEnabled();
            list.append(map);
        }
    }
    
    return list;
}

HealthState SubsystemManager::getSystemHealthState() const
{
    return m_systemHealthState;
}

QString SubsystemManager::getSystemHealthStateString() const
{
    return healthStateToString(m_systemHealthState);
}

double SubsystemManager::getSystemHealthScore() const
{
    return m_systemHealthScore;
}

QVariantMap SubsystemManager::getSystemHealthSummary() const
{
    QVariantMap summary;
    
    summary["state"] = getSystemHealthStateString();
    summary["score"] = m_systemHealthScore;
    summary["totalSubsystems"] = getTotalSubsystemCount();
    summary["activeSubsystems"] = getActiveSubsystemCount();
    summary["healthyCount"] = getHealthySubsystemCount();
    summary["degradedCount"] = getDegradedSubsystemCount();
    summary["failedCount"] = getFailedSubsystemCount();
    summary["totalFaults"] = m_faultManager->getTotalActiveFaults();
    summary["criticalFaults"] = m_faultManager->getCriticalFaultCount();
    
    return summary;
}

QVariantList SubsystemManager::getSubsystemsVariant() const
{
    QVariantList list;
    
    for (auto* subsystem : m_subsystems) {
        QVariantMap map;
        map["id"] = subsystem->getId();
        map["name"] = subsystem->getName();
        map["type"] = subsystem->getTypeName();
        map["description"] = subsystem->getDescription();
        map["healthState"] = subsystem->getHealthStateString();
        map["healthScore"] = subsystem->getHealthScore();
        map["statusMessage"] = subsystem->getStatusMessage();
        map["faultCount"] = subsystem->getFaultCount();
        map["enabled"] = subsystem->isEnabled();
        map["onCanvas"] = m_activeSubsystemIds.contains(subsystem->getId());
        list.append(map);
    }
    
    return list;
}

QVariant SubsystemManager::getSubsystemById(const QString& id) const
{
    RadarSubsystem* sub = m_subsystems.value(id);
    if (!sub) {
        return QVariant();
    }
    
    QVariantMap map;
    map["id"] = sub->getId();
    map["name"] = sub->getName();
    map["type"] = sub->getTypeName();
    map["description"] = sub->getDescription();
    map["healthState"] = sub->getHealthStateString();
    map["healthScore"] = sub->getHealthScore();
    map["statusMessage"] = sub->getStatusMessage();
    map["telemetry"] = sub->getTelemetry();
    map["faults"] = sub->getFaults();
    map["faultCount"] = sub->getFaultCount();
    map["enabled"] = sub->isEnabled();
    
    return map;
}

QVariantList SubsystemManager::getSubsystemsByTypeVariant(const QString& typeName) const
{
    QVariantList list;
    
    for (auto* subsystem : m_subsystems) {
        if (subsystem->getTypeName() == typeName) {
            QVariantMap map;
            map["id"] = subsystem->getId();
            map["name"] = subsystem->getName();
            map["healthState"] = subsystem->getHealthStateString();
            list.append(map);
        }
    }
    
    return list;
}

int SubsystemManager::getTotalSubsystemCount() const
{
    return m_subsystems.size();
}

int SubsystemManager::getActiveSubsystemCount() const
{
    return m_activeSubsystemIds.size();
}

int SubsystemManager::getHealthySubsystemCount() const
{
    int count = 0;
    for (const QString& id : m_activeSubsystemIds) {
        if (RadarSubsystem* sub = m_subsystems.value(id)) {
            if (sub->getHealthState() == HealthState::OK) {
                count++;
            }
        }
    }
    return count;
}

int SubsystemManager::getDegradedSubsystemCount() const
{
    int count = 0;
    for (const QString& id : m_activeSubsystemIds) {
        if (RadarSubsystem* sub = m_subsystems.value(id)) {
            if (sub->getHealthState() == HealthState::DEGRADED) {
                count++;
            }
        }
    }
    return count;
}

int SubsystemManager::getFailedSubsystemCount() const
{
    int count = 0;
    for (const QString& id : m_activeSubsystemIds) {
        if (RadarSubsystem* sub = m_subsystems.value(id)) {
            if (sub->getHealthState() == HealthState::FAIL) {
                count++;
            }
        }
    }
    return count;
}

FaultManager* SubsystemManager::getFaultManager() const
{
    return m_faultManager;
}

void SubsystemManager::setUpdateInterval(int msec)
{
    m_updateInterval = msec;
    if (m_updateTimer->isActive()) {
        m_updateTimer->setInterval(msec);
    }
}

int SubsystemManager::getUpdateInterval() const
{
    return m_updateInterval;
}

void SubsystemManager::startUpdates()
{
    if (!m_updateTimer->isActive()) {
        m_updateTimer->start(m_updateInterval);
    }
}

void SubsystemManager::stopUpdates()
{
    m_updateTimer->stop();
}

void SubsystemManager::updateAll()
{
    for (auto* subsystem : m_subsystems) {
        subsystem->processHealthData();
    }
    computeSystemHealth();
}

void SubsystemManager::resetAll()
{
    for (auto* subsystem : m_subsystems) {
        subsystem->reset();
    }
    m_faultManager->clearAllFaults();
    computeSystemHealth();
}

void SubsystemManager::onSubsystemHealthChanged()
{
    RadarSubsystem* subsystem = qobject_cast<RadarSubsystem*>(sender());
    if (subsystem) {
        emit subsystemHealthChanged(subsystem->getId());
    }
    computeSystemHealth();
}

void SubsystemManager::onSubsystemFaultOccurred(const QString& faultCode, const QString& description)
{
    RadarSubsystem* subsystem = qobject_cast<RadarSubsystem*>(sender());
    if (subsystem) {
        FaultCode fault;
        fault.code = faultCode;
        fault.description = description;
        fault.subsystemId = subsystem->getId();
        fault.timestamp = QDateTime::currentDateTime();
        fault.active = true;
        fault.severity = FaultSeverity::WARNING;  // Could be determined from fault code
        
        m_faultManager->registerFault(fault);
        emit subsystemFaultOccurred(subsystem->getId(), faultCode);
    }
}

void SubsystemManager::connectSubsystemSignals(RadarSubsystem* subsystem)
{
    connect(subsystem, &RadarSubsystem::healthChanged,
            this, &SubsystemManager::onSubsystemHealthChanged);
    connect(subsystem, &RadarSubsystem::faultOccurred,
            this, &SubsystemManager::onSubsystemFaultOccurred);
    connect(subsystem, &RadarSubsystem::faultCleared,
            this, [this, subsystem](const QString& faultCode) {
                m_faultManager->clearFault(faultCode, subsystem->getId());
            });
}

void SubsystemManager::computeSystemHealth()
{
    if (m_activeSubsystemIds.isEmpty()) {
        m_systemHealthState = HealthState::UNKNOWN;
        m_systemHealthScore = 100.0;
        emit systemHealthChanged();
        return;
    }
    
    // Compute aggregate health
    double totalScore = 0;
    bool hasFailed = false;
    bool hasDegraded = false;
    int enabledCount = 0;
    
    for (const QString& id : m_activeSubsystemIds) {
        if (RadarSubsystem* sub = m_subsystems.value(id)) {
            if (!sub->isEnabled()) {
                continue;
            }
            
            enabledCount++;
            totalScore += sub->getHealthScore();
            
            switch (sub->getHealthState()) {
                case HealthState::FAIL:
                    hasFailed = true;
                    break;
                case HealthState::DEGRADED:
                    hasDegraded = true;
                    break;
                default:
                    break;
            }
        }
    }
    
    // Determine overall state
    if (hasFailed) {
        m_systemHealthState = HealthState::FAIL;
    } else if (hasDegraded) {
        m_systemHealthState = HealthState::DEGRADED;
    } else if (enabledCount > 0) {
        m_systemHealthState = HealthState::OK;
    } else {
        m_systemHealthState = HealthState::UNKNOWN;
    }
    
    // Compute average score
    m_systemHealthScore = enabledCount > 0 ? totalScore / enabledCount : 100.0;
    
    emit systemHealthChanged();
}

} // namespace RadarRMP
