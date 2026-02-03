#ifndef RADARSUBSYSTEM_H
#define RADARSUBSYSTEM_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include "IRadarSubsystem.h"
#include "TelemetryData.h"

namespace RadarRMP {

/**
 * @brief Base implementation of radar subsystem
 * 
 * This class provides a concrete base implementation of the IRadarSubsystem
 * interface. Specific subsystems (TX, RX, etc.) inherit from this class
 * and override the necessary methods.
 * 
 * Features:
 * - Thread-safe telemetry and fault management
 * - Automatic health state computation
 * - Signal emission for QML binding
 * - Configurable update intervals
 */
class RadarSubsystem : public QObject, public IRadarSubsystem {
    Q_OBJECT
    
    // QML Properties
    Q_PROPERTY(QString id READ getId CONSTANT)
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString typeName READ getTypeName CONSTANT)
    Q_PROPERTY(QString description READ getDescription CONSTANT)
    Q_PROPERTY(QString healthState READ getHealthStateString NOTIFY healthChanged)
    Q_PROPERTY(double healthScore READ getHealthScore NOTIFY healthChanged)
    Q_PROPERTY(QString statusMessage READ getStatusMessage NOTIFY healthChanged)
    Q_PROPERTY(QVariantMap telemetry READ getTelemetry NOTIFY telemetryChanged)
    Q_PROPERTY(QVariantList faults READ getFaults NOTIFY faultsChanged)
    Q_PROPERTY(int faultCount READ getFaultCount NOTIFY faultsChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool hasFaults READ hasFaults NOTIFY faultsChanged)
    
public:
    explicit RadarSubsystem(const QString& id, const QString& name, 
                           SubsystemType type, QObject* parent = nullptr);
    ~RadarSubsystem() override;
    
    // IRadarSubsystem implementation
    QString getId() const override;
    QString getName() const override;
    SubsystemType getType() const override;
    QString getDescription() const override;
    
    HealthState getHealthState() const override;
    QString getHealthStateString() const override;
    HealthSnapshot getHealthSnapshot() const override;
    double getHealthScore() const override;
    QString getStatusMessage() const override;
    
    QVariantMap getTelemetry() const override;
    QVariant getTelemetryValue(const QString& paramName) const override;
    QStringList getTelemetryParameters() const override;
    QVariantMap getTelemetryMetadata(const QString& paramName) const override;
    
    QVariantList getFaults() const override;
    QVariantList getFaultHistory(int maxCount = 100) const override;
    bool hasFaults() const override;
    int getFaultCount() const override;
    bool clearFault(const QString& faultCode) override;
    int clearAllFaults() override;
    
    bool isEnabled() const override;
    void setEnabled(bool enabled) override;
    void reset() override;
    bool runSelfTest() override;
    
    void updateData(const QVariantMap& data) override;
    void processHealthData() override;
    
    // Additional methods
    QString getTypeName() const;
    void setDescription(const QString& desc);
    
public slots:
    void onUpdate();
    
signals:
    void healthChanged();
    void telemetryChanged();
    void faultsChanged();
    void enabledChanged();
    void faultOccurred(const QString& faultCode, const QString& description);
    void faultCleared(const QString& faultCode);
    void stateTransition(const QString& fromState, const QString& toState);
    
protected:
    // Override in derived classes for subsystem-specific behavior
    virtual void initializeTelemetryParameters();
    virtual HealthState computeHealthState() const;
    virtual double computeHealthScore() const;
    virtual QString computeStatusMessage() const;
    virtual void onDataUpdate(const QVariantMap& data);
    
    // Fault management helpers
    void addFault(const FaultCode& fault);
    void removeFault(const QString& faultCode);
    void updateFault(const QString& faultCode, bool active);
    
    // Telemetry helpers
    void setTelemetryValue(const QString& name, const QVariant& value);
    void addTelemetryParameter(const TelemetryParameter& param);
    
    // State management
    void setHealthState(HealthState state);
    void setStatusMessage(const QString& message);
    
protected:
    QString m_id;
    QString m_name;
    SubsystemType m_type;
    QString m_description;
    
    HealthState m_healthState;
    double m_healthScore;
    QString m_statusMessage;
    
    TelemetryData* m_telemetryData;
    QList<FaultCode> m_activeFaults;
    QList<FaultCode> m_faultHistory;
    
    bool m_enabled;
    mutable QMutex m_mutex;
    
    // Prevent recursive/cascading processHealthData calls
    bool m_processingHealth;
    bool m_healthUpdatePending;
    
    // Signal debouncing to prevent signal storms
    QTimer* m_signalDebounceTimer;
    bool m_pendingHealthSignal;
    bool m_pendingTelemetrySignal;
    qint64 m_lastHealthSignalTime;
    qint64 m_lastTelemetrySignalTime;
    static constexpr int SIGNAL_DEBOUNCE_MS = 50;  // Minimum 50ms between signals
    
    static constexpr int MAX_FAULT_HISTORY = 1000;
};

} // namespace RadarRMP

#endif // RADARSUBSYSTEM_H
