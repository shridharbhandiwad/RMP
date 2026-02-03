#include "core/SubsystemManager.h"
#include <QCoreApplication>

namespace RadarRMP {

SubsystemManager::SubsystemManager(QObject* parent)
    : QObject(parent)
    , m_systemHealthState(HealthState::UNKNOWN)
    , m_systemHealthScore(100.0)
    , m_cachedHealthyCount(0)
    , m_cachedDegradedCount(0)
    , m_cachedFailedCount(0)
    , m_updateInterval(100)  // 100ms throttle interval
    , m_healthUpdatePending(false)
{
    m_faultManager = new FaultManager(this);
    
    // Create models
    m_subsystemModel = new SubsystemListModel(this);
    m_activeModel = new ActiveSubsystemModel(this);
    m_activeModel->setSourceModel(m_subsystemModel);
    
    // Connect active model count changes
    connect(m_activeModel, &ActiveSubsystemModel::countChanged,
            this, &SubsystemManager::activeSubsystemsChanged);
    
    // Throttle timer - coalesces rapid updates into single UI refresh
    m_throttleTimer = new QTimer(this);
    m_throttleTimer->setSingleShot(true);
    connect(m_throttleTimer, &QTimer::timeout, this, &SubsystemManager::onThrottledUpdate);
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
    
    // Add to model
    m_subsystemModel->addSubsystem(subsystem);
    
    connectSubsystemSignals(subsystem);
    
    emit subsystemsChanged();
    scheduleHealthUpdate();
}

void SubsystemManager::unregisterSubsystem(const QString& id)
{
    if (!m_subsystems.contains(id)) {
        return;
    }
    
    // Remove from active list
    m_activeModel->removeFromCanvas(id);
    
    RadarSubsystem* subsystem = m_subsystems.take(id);
    
    // Remove from model
    m_subsystemModel->removeSubsystem(id);
    
    // Clear any active faults
    m_faultManager->clearAllFaults(id);
    
    // Don't delete if it has a different parent
    if (subsystem->parent() == this) {
        subsystem->deleteLater();
    }
    
    emit subsystemsChanged();
    scheduleHealthUpdate();
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
    
    m_activeModel->addToCanvas(subsystemId);
    scheduleHealthUpdate();
}

void SubsystemManager::removeFromCanvas(const QString& subsystemId)
{
    m_activeModel->removeFromCanvas(subsystemId);
    scheduleHealthUpdate();
}

bool SubsystemManager::isOnCanvas(const QString& subsystemId) const
{
    return m_subsystemModel->isOnCanvas(subsystemId);
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
    summary["healthyCount"] = m_cachedHealthyCount;
    summary["degradedCount"] = m_cachedDegradedCount;
    summary["failedCount"] = m_cachedFailedCount;
    summary["totalFaults"] = m_faultManager->getTotalActiveFaults();
    summary["criticalFaults"] = m_faultManager->getCriticalFaultCount();
    
    return summary;
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
    return m_activeModel->count();
}

int SubsystemManager::getHealthySubsystemCount() const
{
    return m_cachedHealthyCount;
}

int SubsystemManager::getDegradedSubsystemCount() const
{
    return m_cachedDegradedCount;
}

int SubsystemManager::getFailedSubsystemCount() const
{
    return m_cachedFailedCount;
}

FaultManager* SubsystemManager::getFaultManager() const
{
    return m_faultManager;
}

void SubsystemManager::setUpdateInterval(int msec)
{
    m_updateInterval = qMax(50, msec);  // Minimum 50ms throttle
}

int SubsystemManager::getUpdateInterval() const
{
    return m_updateInterval;
}

void SubsystemManager::startUpdates()
{
    // No-op now - updates are driven by HealthSimulator and throttled
}

void SubsystemManager::stopUpdates()
{
    m_throttleTimer->stop();
}

void SubsystemManager::scheduleHealthUpdate()
{
    // Mark that an update is needed
    m_healthUpdatePending = true;
    
    // CRITICAL FIX: Only start timer if not already active
    // This prevents timer restart spam which was causing event queue buildup
    // Previous behavior: Timer would restart on EVERY call, delaying actual processing
    // New behavior: Once scheduled, we wait for the timer to fire naturally
    if (!m_throttleTimer->isActive()) {
        m_throttleTimer->start(m_updateInterval);
    }
    // If timer is already running, the pending flag ensures we process when it fires
}

void SubsystemManager::onThrottledUpdate()
{
    if (!m_healthUpdatePending) {
        return;
    }
    
    m_healthUpdatePending = false;
    
    // Compute health state
    computeSystemHealth();
    
    // OPTIMIZATION: Only refresh models if there are active subsystems
    // Refreshing empty models was wasting CPU cycles
    if (m_activeModel->count() > 0) {
        m_subsystemModel->refreshAll();
    }
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
        m_activeModel->refreshSubsystem(subsystem->getId());
    }
    scheduleHealthUpdate();
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
        fault.severity = FaultSeverity::WARNING;
        
        m_faultManager->registerFault(fault);
        emit subsystemFaultOccurred(subsystem->getId(), faultCode);
    }
}

void SubsystemManager::connectSubsystemSignals(RadarSubsystem* subsystem)
{
    // CRITICAL: Use QueuedConnection to prevent blocking and allow debouncing to work
    // DirectConnection would bypass debouncing and cause immediate cascades
    connect(subsystem, &RadarSubsystem::healthChanged,
            this, &SubsystemManager::onSubsystemHealthChanged,
            Qt::QueuedConnection);
    connect(subsystem, &RadarSubsystem::faultOccurred,
            this, &SubsystemManager::onSubsystemFaultOccurred,
            Qt::QueuedConnection);
    connect(subsystem, &RadarSubsystem::faultCleared,
            this, [this, subsystem](const QString& faultCode) {
                // Process in next event loop iteration to prevent blocking
                QMetaObject::invokeMethod(this, [this, subsystem, faultCode]() {
                    m_faultManager->clearFault(faultCode, subsystem->getId());
                }, Qt::QueuedConnection);
            },
            Qt::QueuedConnection);
}

void SubsystemManager::computeSystemHealth()
{
    // Update cached counts
    updateCachedCounts();
    
    int activeCount = m_activeModel->count();
    
    if (activeCount == 0) {
        m_systemHealthState = HealthState::UNKNOWN;
        m_systemHealthScore = 100.0;
        emit systemHealthChanged();
        return;
    }
    
    // Compute aggregate health
    double totalScore = 0;
    int enabledCount = 0;
    
    for (int i = 0; i < m_subsystemModel->rowCount(); ++i) {
        RadarSubsystem* sub = m_subsystemModel->getSubsystem(i);
        if (!sub || !m_subsystemModel->isOnCanvas(sub->getId())) {
            continue;
        }
        
        if (!sub->isEnabled()) {
            continue;
        }
        
        enabledCount++;
        totalScore += sub->getHealthScore();
    }
    
    // Determine overall state based on cached counts
    HealthState newState;
    if (m_cachedFailedCount > 0) {
        newState = HealthState::FAIL;
    } else if (m_cachedDegradedCount > 0) {
        newState = HealthState::DEGRADED;
    } else if (enabledCount > 0) {
        newState = HealthState::OK;
    } else {
        newState = HealthState::UNKNOWN;
    }
    
    // Compute average score
    double newScore = enabledCount > 0 ? totalScore / enabledCount : 100.0;
    
    // Only emit if changed
    if (newState != m_systemHealthState || qAbs(newScore - m_systemHealthScore) > 0.01) {
        m_systemHealthState = newState;
        m_systemHealthScore = newScore;
        emit systemHealthChanged();
    }
}

void SubsystemManager::updateCachedCounts()
{
    int healthy = 0;
    int degraded = 0;
    int failed = 0;
    
    for (int i = 0; i < m_subsystemModel->rowCount(); ++i) {
        RadarSubsystem* sub = m_subsystemModel->getSubsystem(i);
        if (!sub || !m_subsystemModel->isOnCanvas(sub->getId())) {
            continue;
        }
        
        switch (sub->getHealthState()) {
            case HealthState::OK:
                healthy++;
                break;
            case HealthState::DEGRADED:
                degraded++;
                break;
            case HealthState::FAIL:
                failed++;
                break;
            default:
                break;
        }
    }
    
    m_cachedHealthyCount = healthy;
    m_cachedDegradedCount = degraded;
    m_cachedFailedCount = failed;
}

} // namespace RadarRMP
