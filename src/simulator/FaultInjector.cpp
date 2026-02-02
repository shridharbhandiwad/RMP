#include "simulator/FaultInjector.h"
#include "core/SubsystemManager.h"
#include "core/RadarSubsystem.h"

namespace RadarRMP {

FaultInjector::FaultInjector(SubsystemManager* manager, QObject* parent)
    : QObject(parent)
    , m_manager(manager)
{
    initializePredefinedScenarios();
}

FaultInjector::~FaultInjector()
{
    clearAllFaults();
}

void FaultInjector::injectFault(const QString& subsystemId, const QString& faultCode)
{
    FaultConfig config;
    config.subsystemId = subsystemId;
    config.faultCode = faultCode;
    config.severity = FaultSeverity::WARNING;
    config.durationMs = 0;  // Permanent
    config.delayMs = 0;
    config.autoRecover = false;
    
    doInjectFault(config);
}

void FaultInjector::injectFaultWithConfig(const QVariantMap& configMap)
{
    FaultConfig config;
    config.subsystemId = configMap.value("subsystemId").toString();
    config.faultCode = configMap.value("faultCode").toString();
    config.severity = static_cast<FaultSeverity>(configMap.value("severity", 1).toInt());
    config.durationMs = configMap.value("durationMs", 0).toInt();
    config.delayMs = configMap.value("delayMs", 0).toInt();
    config.autoRecover = configMap.value("autoRecover", false).toBool();
    config.affectedTelemetry = configMap.value("affectedTelemetry").toMap();
    
    if (config.delayMs > 0) {
        scheduleFault(config.subsystemId, config.faultCode, config.delayMs);
    } else {
        doInjectFault(config);
    }
}

void FaultInjector::scheduleFault(const QString& subsystemId, const QString& faultCode, int delayMs)
{
    FaultConfig config;
    config.subsystemId = subsystemId;
    config.faultCode = faultCode;
    config.severity = FaultSeverity::WARNING;
    config.durationMs = 0;
    config.delayMs = delayMs;
    config.autoRecover = false;
    
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    m_scheduledFaults[timer] = config;
    
    connect(timer, &QTimer::timeout, this, &FaultInjector::onScheduledFault);
    timer->start(delayMs);
}

void FaultInjector::clearFault(const QString& subsystemId, const QString& faultCode)
{
    QString key = makeFaultKey(subsystemId, faultCode);
    
    if (m_activeFaults.remove(key) > 0) {
        // Clear from subsystem
        if (auto* subsystem = m_manager->getSubsystem(subsystemId)) {
            subsystem->clearFault(faultCode);
        }
        
        // Stop recovery timer if any
        if (m_recoveryTimers.contains(key)) {
            m_recoveryTimers[key]->stop();
            m_recoveryTimers[key]->deleteLater();
            m_recoveryTimers.remove(key);
        }
        
        emit faultCleared(subsystemId, faultCode);
        emit faultsChanged();
    }
}

void FaultInjector::clearAllFaults(const QString& subsystemId)
{
    QStringList keysToRemove;
    
    for (auto it = m_activeFaults.begin(); it != m_activeFaults.end(); ++it) {
        if (it.value().subsystemId == subsystemId) {
            keysToRemove.append(it.key());
        }
    }
    
    for (const QString& key : keysToRemove) {
        FaultConfig config = m_activeFaults.take(key);
        
        if (auto* subsystem = m_manager->getSubsystem(subsystemId)) {
            subsystem->clearFault(config.faultCode);
        }
        
        if (m_recoveryTimers.contains(key)) {
            m_recoveryTimers[key]->stop();
            m_recoveryTimers[key]->deleteLater();
            m_recoveryTimers.remove(key);
        }
        
        emit faultCleared(subsystemId, config.faultCode);
    }
    
    if (!keysToRemove.isEmpty()) {
        emit faultsChanged();
    }
}

void FaultInjector::clearAllFaults()
{
    for (auto it = m_activeFaults.begin(); it != m_activeFaults.end(); ++it) {
        if (auto* subsystem = m_manager->getSubsystem(it.value().subsystemId)) {
            subsystem->clearFault(it.value().faultCode);
        }
        emit faultCleared(it.value().subsystemId, it.value().faultCode);
    }
    
    m_activeFaults.clear();
    
    for (auto* timer : m_recoveryTimers) {
        timer->stop();
        timer->deleteLater();
    }
    m_recoveryTimers.clear();
    
    emit faultsChanged();
}

void FaultInjector::runScenario(const QString& scenarioName)
{
    if (!m_scenarios.contains(scenarioName)) {
        return;
    }
    
    emit scenarioStarted(scenarioName);
    
    const QList<FaultConfig>& faults = m_scenarios[scenarioName];
    
    for (const FaultConfig& config : faults) {
        if (config.delayMs > 0) {
            scheduleFault(config.subsystemId, config.faultCode, config.delayMs);
        } else {
            doInjectFault(config);
        }
    }
    
    emit scenarioCompleted(scenarioName);
}

QStringList FaultInjector::getAvailableScenarios() const
{
    return m_scenarios.keys();
}

bool FaultInjector::isActive() const
{
    return !m_activeFaults.isEmpty();
}

int FaultInjector::getInjectedFaultCount() const
{
    return m_activeFaults.size();
}

QVariantList FaultInjector::getInjectedFaultsVariant() const
{
    QVariantList list;
    
    for (const FaultConfig& config : m_activeFaults) {
        QVariantMap map;
        map["subsystemId"] = config.subsystemId;
        map["faultCode"] = config.faultCode;
        map["severity"] = faultSeverityToString(config.severity);
        list.append(map);
    }
    
    return list;
}

QList<FaultInjector::FaultConfig> FaultInjector::getInjectedFaults() const
{
    return m_activeFaults.values();
}

bool FaultInjector::hasFault(const QString& subsystemId, const QString& faultCode) const
{
    return m_activeFaults.contains(makeFaultKey(subsystemId, faultCode));
}

void FaultInjector::onScheduledFault()
{
    QTimer* timer = qobject_cast<QTimer*>(sender());
    if (!timer || !m_scheduledFaults.contains(timer)) {
        return;
    }
    
    FaultConfig config = m_scheduledFaults.take(timer);
    timer->deleteLater();
    
    doInjectFault(config);
}

void FaultInjector::onFaultTimeout()
{
    QTimer* timer = qobject_cast<QTimer*>(sender());
    if (!timer) {
        return;
    }
    
    // Find and clear the fault
    for (auto it = m_recoveryTimers.begin(); it != m_recoveryTimers.end(); ++it) {
        if (it.value() == timer) {
            QString key = it.key();
            if (m_activeFaults.contains(key)) {
                FaultConfig config = m_activeFaults[key];
                clearFault(config.subsystemId, config.faultCode);
            }
            break;
        }
    }
}

void FaultInjector::doInjectFault(const FaultConfig& config)
{
    QString key = makeFaultKey(config.subsystemId, config.faultCode);
    
    if (m_activeFaults.contains(key)) {
        return;  // Already active
    }
    
    m_activeFaults[key] = config;
    
    // Inject into subsystem
    if (auto* subsystem = m_manager->getSubsystem(config.subsystemId)) {
        FaultCode fault;
        fault.code = config.faultCode;
        fault.description = QString("Injected fault: %1").arg(config.faultCode);
        fault.severity = config.severity;
        fault.subsystemId = config.subsystemId;
        fault.timestamp = QDateTime::currentDateTime();
        fault.active = true;
        
        // Use reflection to add fault
        QMetaObject::invokeMethod(subsystem, "addFault",
                                 Q_ARG(RadarRMP::FaultCode, fault));
    }
    
    // Set up auto-recovery if configured
    if (config.autoRecover && config.durationMs > 0) {
        QTimer* timer = new QTimer(this);
        timer->setSingleShot(true);
        m_recoveryTimers[key] = timer;
        
        connect(timer, &QTimer::timeout, this, &FaultInjector::onFaultTimeout);
        timer->start(config.durationMs);
    }
    
    emit faultInjected(config.subsystemId, config.faultCode);
    emit faultsChanged();
}

void FaultInjector::initializePredefinedScenarios()
{
    // Transmitter failure scenario
    {
        QList<FaultConfig> faults;
        FaultConfig f1;
        f1.subsystemId = "TX-001";
        f1.faultCode = "TX-004";  // Overtemp
        f1.severity = FaultSeverity::CRITICAL;
        f1.durationMs = 30000;
        f1.autoRecover = true;
        faults.append(f1);
        m_scenarios["TransmitterOverheat"] = faults;
    }
    
    // Power failure scenario
    {
        QList<FaultConfig> faults;
        FaultConfig f1;
        f1.subsystemId = "PSU-001";
        f1.faultCode = "PSU-001";  // Input low
        f1.severity = FaultSeverity::CRITICAL;
        f1.durationMs = 0;
        faults.append(f1);
        m_scenarios["PowerFailure"] = faults;
    }
    
    // GPS loss scenario
    {
        QList<FaultConfig> faults;
        FaultConfig f1;
        f1.subsystemId = "TIME-001";
        f1.faultCode = "TIME-001";  // GPS unlock
        f1.severity = FaultSeverity::CRITICAL;
        f1.durationMs = 60000;
        f1.autoRecover = true;
        faults.append(f1);
        m_scenarios["GPSLoss"] = faults;
    }
    
    // Network degradation
    {
        QList<FaultConfig> faults;
        FaultConfig f1;
        f1.subsystemId = "NET-001";
        f1.faultCode = "NET-002";  // High packet loss
        f1.severity = FaultSeverity::WARNING;
        faults.append(f1);
        m_scenarios["NetworkDegradation"] = faults;
    }
    
    // Multiple subsystem failure
    {
        QList<FaultConfig> faults;
        
        FaultConfig f1;
        f1.subsystemId = "TX-001";
        f1.faultCode = "TX-003";  // VSWR
        f1.severity = FaultSeverity::WARNING;
        f1.delayMs = 0;
        faults.append(f1);
        
        FaultConfig f2;
        f2.subsystemId = "COOL-001";
        f2.faultCode = "COOL-001";  // Coolant temp
        f2.severity = FaultSeverity::WARNING;
        f2.delayMs = 5000;
        faults.append(f2);
        
        FaultConfig f3;
        f3.subsystemId = "SP-001";
        f3.faultCode = "SP-001";  // CPU overload
        f3.severity = FaultSeverity::WARNING;
        f3.delayMs = 10000;
        faults.append(f3);
        
        m_scenarios["CascadingFailure"] = faults;
    }
}

QString FaultInjector::makeFaultKey(const QString& subsystemId, const QString& faultCode) const
{
    return subsystemId + ":" + faultCode;
}

} // namespace RadarRMP
