#ifndef SUBSYSTEMMANAGER_H
#define SUBSYSTEMMANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QTimer>
#include "RadarSubsystem.h"
#include "FaultManager.h"

namespace RadarRMP {

/**
 * @brief Central manager for all radar subsystems
 * 
 * The SubsystemManager is the main entry point for the backend.
 * It manages subsystem lifecycle, provides a unified API for QML,
 * and coordinates health data updates.
 */
class SubsystemManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList subsystems READ getSubsystemsVariant NOTIFY subsystemsChanged)
    Q_PROPERTY(QVariantList activeSubsystems READ getActiveSubsystemsVariant NOTIFY activeSubsystemsChanged)
    Q_PROPERTY(QString systemHealthState READ getSystemHealthStateString NOTIFY systemHealthChanged)
    Q_PROPERTY(double systemHealthScore READ getSystemHealthScore NOTIFY systemHealthChanged)
    Q_PROPERTY(int totalSubsystemCount READ getTotalSubsystemCount NOTIFY subsystemsChanged)
    Q_PROPERTY(int activeSubsystemCount READ getActiveSubsystemCount NOTIFY activeSubsystemsChanged)
    Q_PROPERTY(int healthySubsystemCount READ getHealthySubsystemCount NOTIFY systemHealthChanged)
    Q_PROPERTY(int degradedSubsystemCount READ getDegradedSubsystemCount NOTIFY systemHealthChanged)
    Q_PROPERTY(int failedSubsystemCount READ getFailedSubsystemCount NOTIFY systemHealthChanged)
    Q_PROPERTY(FaultManager* faultManager READ getFaultManager CONSTANT)
    
public:
    explicit SubsystemManager(QObject* parent = nullptr);
    ~SubsystemManager() override;
    
    // Subsystem management
    void registerSubsystem(RadarSubsystem* subsystem);
    void unregisterSubsystem(const QString& id);
    RadarSubsystem* getSubsystem(const QString& id) const;
    QList<RadarSubsystem*> getAllSubsystems() const;
    QList<RadarSubsystem*> getSubsystemsByType(SubsystemType type) const;
    
    // Active subsystems (on canvas)
    Q_INVOKABLE void addToCanvas(const QString& subsystemId);
    Q_INVOKABLE void removeFromCanvas(const QString& subsystemId);
    Q_INVOKABLE bool isOnCanvas(const QString& subsystemId) const;
    Q_INVOKABLE QVariantList getActiveSubsystemsVariant() const;
    
    // System health
    HealthState getSystemHealthState() const;
    QString getSystemHealthStateString() const;
    double getSystemHealthScore() const;
    QVariantMap getSystemHealthSummary() const;
    
    // QML helpers
    QVariantList getSubsystemsVariant() const;
    Q_INVOKABLE QVariant getSubsystemById(const QString& id) const;
    Q_INVOKABLE QVariantList getSubsystemsByTypeVariant(const QString& typeName) const;
    
    // Counts
    int getTotalSubsystemCount() const;
    int getActiveSubsystemCount() const;
    int getHealthySubsystemCount() const;
    int getDegradedSubsystemCount() const;
    int getFailedSubsystemCount() const;
    
    // Fault manager access
    FaultManager* getFaultManager() const;
    
    // Update control
    void setUpdateInterval(int msec);
    int getUpdateInterval() const;
    
public slots:
    void startUpdates();
    void stopUpdates();
    void updateAll();
    void resetAll();
    
signals:
    void subsystemsChanged();
    void activeSubsystemsChanged();
    void systemHealthChanged();
    void subsystemHealthChanged(const QString& subsystemId);
    void subsystemFaultOccurred(const QString& subsystemId, const QString& faultCode);
    
private slots:
    void onSubsystemHealthChanged();
    void onSubsystemFaultOccurred(const QString& faultCode, const QString& description);
    
private:
    void connectSubsystemSignals(RadarSubsystem* subsystem);
    void computeSystemHealth();
    
    QMap<QString, RadarSubsystem*> m_subsystems;
    QList<QString> m_activeSubsystemIds;
    FaultManager* m_faultManager;
    
    HealthState m_systemHealthState;
    double m_systemHealthScore;
    
    QTimer* m_updateTimer;
    int m_updateInterval;
};

} // namespace RadarRMP

#endif // SUBSYSTEMMANAGER_H
