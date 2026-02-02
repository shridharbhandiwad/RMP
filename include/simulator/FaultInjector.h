#ifndef FAULTINJECTOR_H
#define FAULTINJECTOR_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QDateTime>
#include "core/HealthStatus.h"

namespace RadarRMP {

class SubsystemManager;

/**
 * @brief Controlled fault injection for testing and training
 * 
 * Allows precise control over fault injection for:
 * - System testing
 * - Operator training
 * - Failure mode analysis
 * - Recovery procedure validation
 */
class FaultInjector : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(int injectedFaultCount READ getInjectedFaultCount NOTIFY faultsChanged)
    Q_PROPERTY(QVariantList injectedFaults READ getInjectedFaultsVariant NOTIFY faultsChanged)
    
public:
    /**
     * @brief Fault injection configuration
     */
    struct FaultConfig {
        QString subsystemId;
        QString faultCode;
        FaultSeverity severity;
        int durationMs;        // 0 = permanent until cleared
        int delayMs;           // Delay before injection
        bool autoRecover;      // Automatically clear after duration
        QVariantMap affectedTelemetry;  // Telemetry values to modify
    };
    
    explicit FaultInjector(SubsystemManager* manager, QObject* parent = nullptr);
    ~FaultInjector() override;
    
    // Fault injection
    Q_INVOKABLE void injectFault(const QString& subsystemId, const QString& faultCode);
    Q_INVOKABLE void injectFaultWithConfig(const QVariantMap& config);
    Q_INVOKABLE void scheduleFault(const QString& subsystemId, const QString& faultCode, int delayMs);
    
    // Fault clearing
    Q_INVOKABLE void clearFault(const QString& subsystemId, const QString& faultCode);
    Q_INVOKABLE void clearAllFaults(const QString& subsystemId);
    Q_INVOKABLE void clearAllFaults();
    
    // Predefined fault scenarios
    Q_INVOKABLE void runScenario(const QString& scenarioName);
    Q_INVOKABLE QStringList getAvailableScenarios() const;
    
    // Bulk operations
    void loadFaultScript(const QString& scriptPath);
    void saveFaultScript(const QString& scriptPath) const;
    
    // Status
    bool isActive() const;
    int getInjectedFaultCount() const;
    QVariantList getInjectedFaultsVariant() const;
    QList<FaultConfig> getInjectedFaults() const;
    bool hasFault(const QString& subsystemId, const QString& faultCode) const;
    
signals:
    void activeChanged();
    void faultsChanged();
    void faultInjected(const QString& subsystemId, const QString& faultCode);
    void faultCleared(const QString& subsystemId, const QString& faultCode);
    void scenarioStarted(const QString& scenarioName);
    void scenarioCompleted(const QString& scenarioName);
    
private slots:
    void onScheduledFault();
    void onFaultTimeout();
    
private:
    void doInjectFault(const FaultConfig& config);
    void initializePredefinedScenarios();
    
    SubsystemManager* m_manager;
    
    // Active faults
    QMap<QString, FaultConfig> m_activeFaults;  // Key: subsystemId:faultCode
    
    // Scheduled faults
    QMap<QTimer*, FaultConfig> m_scheduledFaults;
    
    // Auto-recovery timers
    QMap<QString, QTimer*> m_recoveryTimers;
    
    // Predefined scenarios
    QMap<QString, QList<FaultConfig>> m_scenarios;
    
    QString makeFaultKey(const QString& subsystemId, const QString& faultCode) const;
};

} // namespace RadarRMP

#endif // FAULTINJECTOR_H
