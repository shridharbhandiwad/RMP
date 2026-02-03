#include "core/RadarSubsystem.h"
#include <QMutexLocker>
#include <QTimer>
#include <QDateTime>

namespace RadarRMP {

RadarSubsystem::RadarSubsystem(const QString& id, const QString& name, 
                               SubsystemType type, QObject* parent)
    : QObject(parent)
    , m_id(id)
    , m_name(name)
    , m_type(type)
    , m_healthState(HealthState::UNKNOWN)
    , m_healthScore(100.0)
    , m_enabled(true)
    , m_processingHealth(false)
    , m_healthUpdatePending(false)
    , m_pendingHealthSignal(false)
    , m_pendingTelemetrySignal(false)
    , m_lastHealthSignalTime(0)
    , m_lastTelemetrySignalTime(0)
{
    m_telemetryData = new TelemetryData(this);
    
    // NOTE: We no longer automatically trigger processHealthData from telemetry changes
    // This prevents cascading signal emissions. processHealthData is called explicitly
    // after all data updates are complete in updateData().
    // REMOVED direct signal connection to prevent signal storms
    
    m_description = subsystemTypeToString(type);
    
    // Create debounce timer for batched signal emission
    m_signalDebounceTimer = new QTimer(this);
    m_signalDebounceTimer->setSingleShot(true);
    m_signalDebounceTimer->setInterval(SIGNAL_DEBOUNCE_MS);
    connect(m_signalDebounceTimer, &QTimer::timeout, this, [this]() {
        // Emit any pending signals in batch
        if (m_pendingHealthSignal) {
            m_pendingHealthSignal = false;
            m_lastHealthSignalTime = QDateTime::currentMSecsSinceEpoch();
            emit healthChanged();
        }
        if (m_pendingTelemetrySignal) {
            m_pendingTelemetrySignal = false;
            m_lastTelemetrySignalTime = QDateTime::currentMSecsSinceEpoch();
            emit telemetryChanged();
        }
    });
    
    initializeTelemetryParameters();
}

RadarSubsystem::~RadarSubsystem()
{
}

QString RadarSubsystem::getId() const
{
    return m_id;
}

QString RadarSubsystem::getName() const
{
    return m_name;
}

SubsystemType RadarSubsystem::getType() const
{
    return m_type;
}

QString RadarSubsystem::getDescription() const
{
    return m_description;
}

QString RadarSubsystem::getTypeName() const
{
    return subsystemTypeToString(m_type);
}

void RadarSubsystem::setDescription(const QString& desc)
{
    m_description = desc;
}

HealthState RadarSubsystem::getHealthState() const
{
    QMutexLocker locker(&m_mutex);
    return m_healthState;
}

QString RadarSubsystem::getHealthStateString() const
{
    return healthStateToString(getHealthState());
}

HealthSnapshot RadarSubsystem::getHealthSnapshot() const
{
    QMutexLocker locker(&m_mutex);
    
    HealthSnapshot snapshot;
    snapshot.state = m_healthState;
    snapshot.timestamp = QDateTime::currentDateTime();
    snapshot.telemetry = m_telemetryData->getData();
    snapshot.activeFaults = m_activeFaults;
    snapshot.healthScore = m_healthScore;
    snapshot.statusMessage = m_statusMessage;
    
    return snapshot;
}

double RadarSubsystem::getHealthScore() const
{
    QMutexLocker locker(&m_mutex);
    return m_healthScore;
}

QString RadarSubsystem::getStatusMessage() const
{
    QMutexLocker locker(&m_mutex);
    return m_statusMessage;
}

QVariantMap RadarSubsystem::getTelemetry() const
{
    return m_telemetryData->getData();
}

QVariant RadarSubsystem::getTelemetryValue(const QString& paramName) const
{
    return m_telemetryData->getValue(paramName);
}

QStringList RadarSubsystem::getTelemetryParameters() const
{
    return m_telemetryData->getParameterNames();
}

QVariantMap RadarSubsystem::getTelemetryMetadata(const QString& paramName) const
{
    return m_telemetryData->getParameter(paramName).toVariantMap();
}

QVariantList RadarSubsystem::getFaults() const
{
    QMutexLocker locker(&m_mutex);
    QVariantList faults;
    
    for (const auto& fault : m_activeFaults) {
        QVariantMap faultMap;
        faultMap["code"] = fault.code;
        faultMap["description"] = fault.description;
        faultMap["severity"] = faultSeverityToString(fault.severity);
        faultMap["timestamp"] = fault.timestamp;
        faultMap["active"] = fault.active;
        faults.append(faultMap);
    }
    
    return faults;
}

QVariantList RadarSubsystem::getFaultHistory(int maxCount) const
{
    QMutexLocker locker(&m_mutex);
    QVariantList history;
    
    int count = 0;
    for (auto it = m_faultHistory.rbegin(); 
         it != m_faultHistory.rend() && count < maxCount; 
         ++it, ++count) {
        QVariantMap faultMap;
        faultMap["code"] = it->code;
        faultMap["description"] = it->description;
        faultMap["severity"] = faultSeverityToString(it->severity);
        faultMap["timestamp"] = it->timestamp;
        faultMap["active"] = it->active;
        history.append(faultMap);
    }
    
    return history;
}

bool RadarSubsystem::hasFaults() const
{
    QMutexLocker locker(&m_mutex);
    return !m_activeFaults.isEmpty();
}

int RadarSubsystem::getFaultCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_activeFaults.size();
}

bool RadarSubsystem::clearFault(const QString& faultCode)
{
    QMutexLocker locker(&m_mutex);
    
    for (int i = 0; i < m_activeFaults.size(); ++i) {
        if (m_activeFaults[i].code == faultCode) {
            FaultCode fault = m_activeFaults.takeAt(i);
            fault.active = false;
            m_faultHistory.append(fault);
            
            // Trim history if needed
            while (m_faultHistory.size() > MAX_FAULT_HISTORY) {
                m_faultHistory.removeFirst();
            }
            
            locker.unlock();
            emit faultCleared(faultCode);
            emit faultsChanged();
            // Schedule health update instead of immediate call
            QTimer::singleShot(0, this, &RadarSubsystem::processHealthData);
            return true;
        }
    }
    
    return false;
}

int RadarSubsystem::clearAllFaults()
{
    QMutexLocker locker(&m_mutex);
    int count = m_activeFaults.size();
    
    for (auto& fault : m_activeFaults) {
        fault.active = false;
        m_faultHistory.append(fault);
    }
    m_activeFaults.clear();
    
    // Trim history
    while (m_faultHistory.size() > MAX_FAULT_HISTORY) {
        m_faultHistory.removeFirst();
    }
    
    locker.unlock();
    
    if (count > 0) {
        emit faultsChanged();
        // Schedule health update instead of immediate call
        QTimer::singleShot(0, this, &RadarSubsystem::processHealthData);
    }
    
    return count;
}

bool RadarSubsystem::isEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_enabled;
}

void RadarSubsystem::setEnabled(bool enabled)
{
    {
        QMutexLocker locker(&m_mutex);
        if (m_enabled == enabled) {
            return;
        }
        m_enabled = enabled;
    }
    emit enabledChanged();
    // Schedule health update instead of immediate call
    QTimer::singleShot(0, this, &RadarSubsystem::processHealthData);
}

void RadarSubsystem::reset()
{
    QMutexLocker locker(&m_mutex);
    
    m_activeFaults.clear();
    m_healthState = HealthState::UNKNOWN;
    m_healthScore = 100.0;
    m_statusMessage.clear();
    
    locker.unlock();
    
    initializeTelemetryParameters();
    
    emit healthChanged();
    emit faultsChanged();
    emit telemetryChanged();
}

bool RadarSubsystem::runSelfTest()
{
    // Base implementation - override in derived classes
    processHealthData();
    return getHealthState() == HealthState::OK;
}

void RadarSubsystem::updateData(const QVariantMap& data)
{
    m_telemetryData->setValues(data);
    onDataUpdate(data);
    
    // Schedule debounced telemetry signal
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if ((now - m_lastTelemetrySignalTime) >= SIGNAL_DEBOUNCE_MS) {
        // Enough time has passed, emit immediately
        m_lastTelemetrySignalTime = now;
        emit telemetryChanged();
    } else {
        // Too soon, schedule for later
        m_pendingTelemetrySignal = true;
        if (!m_signalDebounceTimer->isActive()) {
            m_signalDebounceTimer->start();
        }
    }
    
    processHealthData();
}

void RadarSubsystem::processHealthData()
{
    // Prevent recursive calls - if already processing, just mark pending
    if (m_processingHealth) {
        m_healthUpdatePending = true;
        return;
    }
    
    m_processingHealth = true;
    
    QMutexLocker locker(&m_mutex);
    
    HealthState oldState = m_healthState;
    double oldScore = m_healthScore;
    
    // Compute new health state
    m_healthState = computeHealthState();
    m_healthScore = computeHealthScore();
    m_statusMessage = computeStatusMessage();
    
    locker.unlock();
    
    // Only emit signals if something actually changed
    bool stateChanged = (oldState != m_healthState);
    bool scoreChanged = qAbs(oldScore - m_healthScore) > 0.1;  // 0.1% threshold
    
    if (stateChanged) {
        emit stateTransition(healthStateToString(oldState), 
                            healthStateToString(m_healthState));
    }
    
    if (stateChanged || scoreChanged) {
        // Use debounced signal emission to prevent storms
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if ((now - m_lastHealthSignalTime) >= SIGNAL_DEBOUNCE_MS) {
            // Enough time has passed, emit immediately
            m_lastHealthSignalTime = now;
            emit healthChanged();
        } else {
            // Too soon, schedule for later
            m_pendingHealthSignal = true;
            if (!m_signalDebounceTimer->isActive()) {
                m_signalDebounceTimer->start();
            }
        }
    }
    
    m_processingHealth = false;
    
    // If another update was requested while we were processing, do it now
    if (m_healthUpdatePending) {
        m_healthUpdatePending = false;
        // Use QTimer::singleShot to prevent deep recursion
        QTimer::singleShot(0, this, &RadarSubsystem::processHealthData);
    }
}

void RadarSubsystem::onUpdate()
{
    processHealthData();
}

void RadarSubsystem::initializeTelemetryParameters()
{
    // Base implementation - override in derived classes
}

HealthState RadarSubsystem::computeHealthState() const
{
    // Default implementation based on faults
    if (!m_enabled) {
        return HealthState::UNKNOWN;
    }
    
    bool hasCritical = false;
    bool hasWarning = false;
    
    for (const auto& fault : m_activeFaults) {
        if (fault.severity == FaultSeverity::FATAL || 
            fault.severity == FaultSeverity::CRITICAL) {
            hasCritical = true;
        } else if (fault.severity == FaultSeverity::WARNING) {
            hasWarning = true;
        }
    }
    
    if (hasCritical) {
        return HealthState::FAIL;
    } else if (hasWarning) {
        return HealthState::DEGRADED;
    }
    
    return HealthState::OK;
}

double RadarSubsystem::computeHealthScore() const
{
    // Default implementation
    double score = 100.0;
    
    for (const auto& fault : m_activeFaults) {
        switch (fault.severity) {
            case FaultSeverity::INFO:
                score -= 5;
                break;
            case FaultSeverity::WARNING:
                score -= 15;
                break;
            case FaultSeverity::CRITICAL:
                score -= 30;
                break;
            case FaultSeverity::FATAL:
                score -= 50;
                break;
        }
    }
    
    return qMax(0.0, score);
}

QString RadarSubsystem::computeStatusMessage() const
{
    if (!m_enabled) {
        return "Disabled";
    }
    
    if (m_activeFaults.isEmpty()) {
        return "Operating normally";
    }
    
    // Return the most severe fault description
    FaultSeverity maxSeverity = FaultSeverity::INFO;
    QString message = "Active faults present";
    
    for (const auto& fault : m_activeFaults) {
        if (fault.severity > maxSeverity) {
            maxSeverity = fault.severity;
            message = fault.description;
        }
    }
    
    return message;
}

void RadarSubsystem::onDataUpdate(const QVariantMap& data)
{
    // Override in derived classes
    Q_UNUSED(data)
}

void RadarSubsystem::addFault(const FaultCode& fault)
{
    QMutexLocker locker(&m_mutex);
    
    // Check if fault already exists
    for (const auto& existing : m_activeFaults) {
        if (existing.code == fault.code) {
            return;
        }
    }
    
    m_activeFaults.append(fault);
    
    locker.unlock();
    
    emit faultOccurred(fault.code, fault.description);
    emit faultsChanged();
    // Schedule health update instead of immediate call
    QTimer::singleShot(0, this, &RadarSubsystem::processHealthData);
}

void RadarSubsystem::removeFault(const QString& faultCode)
{
    clearFault(faultCode);
}

void RadarSubsystem::updateFault(const QString& faultCode, bool active)
{
    if (active) {
        // If activating, check if it already exists
        QMutexLocker locker(&m_mutex);
        for (const auto& fault : m_activeFaults) {
            if (fault.code == faultCode) {
                return;  // Already active
            }
        }
    } else {
        clearFault(faultCode);
    }
}

void RadarSubsystem::setTelemetryValue(const QString& name, const QVariant& value)
{
    m_telemetryData->setValue(name, value);
}

void RadarSubsystem::addTelemetryParameter(const TelemetryParameter& param)
{
    m_telemetryData->addParameter(param);
}

void RadarSubsystem::setHealthState(HealthState state)
{
    QMutexLocker locker(&m_mutex);
    HealthState oldState = m_healthState;
    m_healthState = state;
    locker.unlock();
    
    if (oldState != state) {
        emit stateTransition(healthStateToString(oldState), 
                            healthStateToString(state));
        emit healthChanged();
    }
}

void RadarSubsystem::setStatusMessage(const QString& message)
{
    QMutexLocker locker(&m_mutex);
    m_statusMessage = message;
    locker.unlock();
    emit healthChanged();
}

} // namespace RadarRMP
