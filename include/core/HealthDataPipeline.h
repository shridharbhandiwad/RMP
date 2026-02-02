#ifndef HEALTHDATAPIPELINE_H
#define HEALTHDATAPIPELINE_H

#include <QObject>
#include <QVariantMap>
#include <QQueue>
#include <QMutex>
#include <QThread>
#include "HealthStatus.h"

namespace RadarRMP {

class RadarSubsystem;

/**
 * @brief Health data processing pipeline
 * 
 * Implements the data flow: Input → Validation → Status computation → UI update
 * 
 * The pipeline processes incoming health data, validates it against defined
 * schemas and thresholds, computes derived health metrics, and propagates
 * updates to the UI layer.
 */
class HealthDataPipeline : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(int queuedItems READ getQueuedItemCount NOTIFY queueChanged)
    Q_PROPERTY(int processedCount READ getProcessedCount NOTIFY statsChanged)
    Q_PROPERTY(int errorCount READ getErrorCount NOTIFY statsChanged)
    
public:
    /**
     * @brief Data validation result
     */
    struct ValidationResult {
        bool valid;
        QString errorMessage;
        QStringList warnings;
        QVariantMap sanitizedData;
        
        ValidationResult() : valid(true) {}
    };
    
    /**
     * @brief Processing stage result
     */
    struct ProcessingResult {
        HealthState computedState;
        double healthScore;
        QVariantMap processedTelemetry;
        QList<FaultCode> detectedFaults;
        QString statusMessage;
    };
    
    explicit HealthDataPipeline(QObject* parent = nullptr);
    ~HealthDataPipeline() override;
    
    // Pipeline control
    void start();
    void stop();
    bool isRunning() const;
    
    // Data submission
    void submitData(const QString& subsystemId, const QVariantMap& data);
    void submitBatchData(const QVariantMap& batchData);
    
    // Processing
    ValidationResult validateData(const QString& subsystemId, const QVariantMap& data) const;
    ProcessingResult processData(const QString& subsystemId, const QVariantMap& data) const;
    
    // Schema management
    void setValidationSchema(const QString& subsystemType, const QVariantMap& schema);
    QVariantMap getValidationSchema(const QString& subsystemType) const;
    
    // Statistics
    int getQueuedItemCount() const;
    int getProcessedCount() const;
    int getErrorCount() const;
    QVariantMap getStatistics() const;
    
    // Threshold configuration
    void setDefaultThresholds(const QVariantMap& thresholds);
    void setSubsystemThresholds(const QString& subsystemId, const QVariantMap& thresholds);
    
signals:
    void runningChanged();
    void queueChanged();
    void statsChanged();
    void dataProcessed(const QString& subsystemId, const QVariantMap& result);
    void validationError(const QString& subsystemId, const QString& error);
    void healthStateComputed(const QString& subsystemId, int state, double score);
    void faultDetected(const QString& subsystemId, const QString& faultCode);
    
private slots:
    void processQueue();
    
private:
    // Validation stages
    bool validateDataTypes(const QVariantMap& data, const QVariantMap& schema) const;
    bool validateRanges(const QVariantMap& data, const QVariantMap& schema) const;
    bool validateRequired(const QVariantMap& data, const QVariantMap& schema) const;
    QVariantMap sanitizeData(const QVariantMap& data) const;
    
    // Processing stages
    HealthState computeHealthState(const QVariantMap& data, const QVariantMap& thresholds) const;
    double computeHealthScore(const QVariantMap& data, const QVariantMap& thresholds) const;
    QList<FaultCode> detectFaults(const QString& subsystemId, const QVariantMap& data, 
                                  const QVariantMap& thresholds) const;
    
    struct QueueItem {
        QString subsystemId;
        QVariantMap data;
        qint64 timestamp;
    };
    
    QQueue<QueueItem> m_dataQueue;
    mutable QMutex m_queueMutex;
    
    QMap<QString, QVariantMap> m_validationSchemas;
    QMap<QString, QVariantMap> m_thresholds;
    QVariantMap m_defaultThresholds;
    
    bool m_running;
    int m_processedCount;
    int m_errorCount;
    
    QThread* m_processingThread;
    QTimer* m_processTimer;
};

} // namespace RadarRMP

#endif // HEALTHDATAPIPELINE_H
