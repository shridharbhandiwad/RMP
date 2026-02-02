#ifndef SUBSYSTEMMANAGER_H
#define SUBSYSTEMMANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QTimer>
#include "RadarSubsystem.h"
#include "FaultManager.h"
#include "SubsystemListModel.h"

namespace RadarRMP {

/**
 * @brief Central manager for all radar subsystems
 * 
 * The SubsystemManager is the main entry point for the backend.
 * It manages subsystem lifecycle, provides a unified API for QML,
 * and coordinates health data updates.
 * 
 * ARCHITECTURE NOTES:
 * - Uses QAbstractListModel for efficient QML binding (no QVariantList recreation)
 * - Employs signal throttling to prevent UI overload
 * - Does NOT run its own update timer (HealthSimulator drives updates)
 * - Batches health computations to reduce redundant calculations
 */
class SubsystemManager : public QObject {
    Q_OBJECT
    
    // Use proper models instead of QVariantList for efficient QML binding
    Q_PROPERTY(SubsystemListModel* subsystemModel READ getSubsystemModel CONSTANT)
    Q_PROPERTY(ActiveSubsystemModel* activeSubsystemModel READ getActiveSubsystemModel CONSTANT)
    
    // Keep simple properties for header displays (cached values)
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
    
    // Model access
    SubsystemListModel* getSubsystemModel() const { return m_subsystemModel; }
    ActiveSubsystemModel* getActiveSubsystemModel() const { return m_activeModel; }
    
    // Active subsystems (on canvas)
    Q_INVOKABLE void addToCanvas(const QString& subsystemId);
    Q_INVOKABLE void removeFromCanvas(const QString& subsystemId);
    Q_INVOKABLE bool isOnCanvas(const QString& subsystemId) const;
    
    // System health
    HealthState getSystemHealthState() const;
    QString getSystemHealthStateString() const;
    double getSystemHealthScore() const;
    QVariantMap getSystemHealthSummary() const;
    
    // QML helpers - these now use cached data
    Q_INVOKABLE QVariant getSubsystemById(const QString& id) const;
    Q_INVOKABLE QVariantList getSubsystemsByTypeVariant(const QString& typeName) const;
    
    // Counts (use cached values)
    int getTotalSubsystemCount() const;
    int getActiveSubsystemCount() const;
    int getHealthySubsystemCount() const;
    int getDegradedSubsystemCount() const;
    int getFailedSubsystemCount() const;
    
    // Fault manager access
    FaultManager* getFaultManager() const;
    
    // Update control - now primarily for throttle interval
    void setUpdateInterval(int msec);
    int getUpdateInterval() const;
    
public slots:
    void startUpdates();
    void stopUpdates();
    void resetAll();
    
    // Called by simulator - schedules batched update
    void scheduleHealthUpdate();
    
signals:
    void subsystemsChanged();
    void activeSubsystemsChanged();
    void systemHealthChanged();
    void subsystemHealthChanged(const QString& subsystemId);
    void subsystemFaultOccurred(const QString& subsystemId, const QString& faultCode);
    
private slots:
    void onSubsystemHealthChanged();
    void onSubsystemFaultOccurred(const QString& faultCode, const QString& description);
    void onThrottledUpdate();
    
private:
    void connectSubsystemSignals(RadarSubsystem* subsystem);
    void computeSystemHealth();
    void updateCachedCounts();
    
    // Subsystem storage
    QMap<QString, RadarSubsystem*> m_subsystems;
    
    // Models for QML
    SubsystemListModel* m_subsystemModel;
    ActiveSubsystemModel* m_activeModel;
    
    FaultManager* m_faultManager;
    
    // Cached health state (avoid recomputation on every access)
    HealthState m_systemHealthState;
    double m_systemHealthScore;
    int m_cachedHealthyCount;
    int m_cachedDegradedCount;
    int m_cachedFailedCount;
    
    // Throttling mechanism - batch updates instead of immediate
    QTimer* m_throttleTimer;
    int m_updateInterval;
    bool m_healthUpdatePending;
};

} // namespace RadarRMP

#endif // SUBSYSTEMMANAGER_H
