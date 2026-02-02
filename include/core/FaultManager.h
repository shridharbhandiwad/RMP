#ifndef FAULTMANAGER_H
#define FAULTMANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QDateTime>
#include <QTimer>
#include "HealthStatus.h"

namespace RadarRMP {

/**
 * @brief System-wide fault manager
 * 
 * Manages fault tracking, history, and statistics across all subsystems.
 * Provides centralized fault logging, correlation, and reporting.
 */
class FaultManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int totalActiveFaults READ getTotalActiveFaults NOTIFY faultsChanged)
    Q_PROPERTY(int criticalFaultCount READ getCriticalFaultCount NOTIFY faultsChanged)
    Q_PROPERTY(QVariantList activeFaults READ getActiveFaultsVariant NOTIFY faultsChanged)
    Q_PROPERTY(QVariantList recentFaults READ getRecentFaultsVariant NOTIFY faultsChanged)
    
public:
    explicit FaultManager(QObject* parent = nullptr);
    ~FaultManager() override = default;
    
    // Fault registration
    void registerFault(const FaultCode& fault);
    void clearFault(const QString& faultCode, const QString& subsystemId);
    void clearAllFaults(const QString& subsystemId);
    void clearAllFaults();
    
    // Fault queries
    QList<FaultCode> getActiveFaults() const;
    QList<FaultCode> getActiveFaults(const QString& subsystemId) const;
    QList<FaultCode> getFaultHistory(int maxCount = 100) const;
    QList<FaultCode> getFaultHistory(const QString& subsystemId, int maxCount = 100) const;
    
    bool hasFault(const QString& faultCode) const;
    FaultCode getFault(const QString& faultCode) const;
    
    // Statistics
    int getTotalActiveFaults() const;
    int getCriticalFaultCount() const;
    int getFaultCount(FaultSeverity severity) const;
    int getFaultCount(const QString& subsystemId) const;
    
    // QML helpers
    QVariantList getActiveFaultsVariant() const;
    QVariantList getRecentFaultsVariant(int maxCount = 10) const;
    QVariantMap getFaultStatistics() const;
    
    // MTBF estimation
    double estimateMTBF(const QString& subsystemId) const;
    QVariantMap getMTBFReport() const;
    
public slots:
    void onSubsystemFault(const QString& subsystemId, const FaultCode& fault);
    void onSubsystemFaultCleared(const QString& subsystemId, const QString& faultCode);
    
signals:
    void faultsChanged();
    void faultRegistered(const QString& subsystemId, const QString& faultCode);
    void faultCleared(const QString& subsystemId, const QString& faultCode);
    void criticalFaultOccurred(const QString& subsystemId, const QString& faultCode);
    
private:
    QMap<QString, FaultCode> m_activeFaults;  // Key: "subsystemId:faultCode"
    QList<FaultCode> m_faultHistory;
    QMap<QString, QDateTime> m_subsystemLastFault;
    QMap<QString, int> m_subsystemFaultCounts;
    
    static constexpr int MAX_HISTORY_SIZE = 10000;
    
    QString makeFaultKey(const QString& subsystemId, const QString& faultCode) const;
};

} // namespace RadarRMP

#endif // FAULTMANAGER_H
