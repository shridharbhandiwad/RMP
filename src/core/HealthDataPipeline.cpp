#include "core/HealthDataPipeline.h"
#include <QTimer>
#include <QThread>
#include <cmath>

namespace RadarRMP {

HealthDataPipeline::HealthDataPipeline(QObject* parent)
    : QObject(parent)
    , m_running(false)
    , m_processedCount(0)
    , m_errorCount(0)
    , m_processingThread(nullptr)
{
    m_processTimer = new QTimer(this);
    m_processTimer->setInterval(10);  // Process every 10ms
    connect(m_processTimer, &QTimer::timeout, this, &HealthDataPipeline::processQueue);
    
    // Initialize default thresholds
    m_defaultThresholds["temperature"] = QVariantMap{
        {"warningHigh", 60.0},
        {"criticalHigh", 80.0}
    };
}

HealthDataPipeline::~HealthDataPipeline()
{
    stop();
}

void HealthDataPipeline::start()
{
    if (m_running) {
        return;
    }
    
    m_running = true;
    m_processTimer->start();
    emit runningChanged();
}

void HealthDataPipeline::stop()
{
    if (!m_running) {
        return;
    }
    
    m_running = false;
    m_processTimer->stop();
    emit runningChanged();
}

bool HealthDataPipeline::isRunning() const
{
    return m_running;
}

void HealthDataPipeline::submitData(const QString& subsystemId, const QVariantMap& data)
{
    QMutexLocker locker(&m_queueMutex);
    
    QueueItem item;
    item.subsystemId = subsystemId;
    item.data = data;
    item.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    m_dataQueue.enqueue(item);
    emit queueChanged();
}

void HealthDataPipeline::submitBatchData(const QVariantMap& batchData)
{
    QMutexLocker locker(&m_queueMutex);
    
    for (auto it = batchData.begin(); it != batchData.end(); ++it) {
        QueueItem item;
        item.subsystemId = it.key();
        item.data = it.value().toMap();
        item.timestamp = QDateTime::currentMSecsSinceEpoch();
        m_dataQueue.enqueue(item);
    }
    
    emit queueChanged();
}

HealthDataPipeline::ValidationResult HealthDataPipeline::validateData(
    const QString& subsystemId, const QVariantMap& data) const
{
    ValidationResult result;
    result.valid = true;
    result.sanitizedData = data;
    
    // Get schema for subsystem type (if any)
    QVariantMap schema = m_validationSchemas.value(subsystemId);
    
    // Validate required fields
    if (!validateRequired(data, schema)) {
        result.valid = false;
        result.errorMessage = "Missing required fields";
        return result;
    }
    
    // Validate data types
    if (!validateDataTypes(data, schema)) {
        result.valid = false;
        result.errorMessage = "Invalid data types";
        return result;
    }
    
    // Validate ranges
    if (!validateRanges(data, schema)) {
        result.warnings.append("Some values outside expected range");
    }
    
    // Sanitize data
    result.sanitizedData = sanitizeData(data);
    
    return result;
}

HealthDataPipeline::ProcessingResult HealthDataPipeline::processData(
    const QString& subsystemId, const QVariantMap& data) const
{
    ProcessingResult result;
    
    // Get thresholds
    QVariantMap thresholds = m_thresholds.value(subsystemId, m_defaultThresholds);
    
    // Process telemetry values
    result.processedTelemetry = data;
    
    // Compute health state
    result.computedState = computeHealthState(data, thresholds);
    
    // Compute health score
    result.healthScore = computeHealthScore(data, thresholds);
    
    // Detect faults
    result.detectedFaults = detectFaults(subsystemId, data, thresholds);
    
    // Generate status message
    if (result.computedState == HealthState::OK) {
        result.statusMessage = "Operating normally";
    } else if (result.computedState == HealthState::DEGRADED) {
        result.statusMessage = "Degraded performance";
    } else if (result.computedState == HealthState::FAIL) {
        result.statusMessage = "System failure";
    } else {
        result.statusMessage = "Status unknown";
    }
    
    return result;
}

void HealthDataPipeline::setValidationSchema(const QString& subsystemType, 
                                              const QVariantMap& schema)
{
    m_validationSchemas[subsystemType] = schema;
}

QVariantMap HealthDataPipeline::getValidationSchema(const QString& subsystemType) const
{
    return m_validationSchemas.value(subsystemType);
}

int HealthDataPipeline::getQueuedItemCount() const
{
    QMutexLocker locker(&m_queueMutex);
    return m_dataQueue.size();
}

int HealthDataPipeline::getProcessedCount() const
{
    return m_processedCount;
}

int HealthDataPipeline::getErrorCount() const
{
    return m_errorCount;
}

QVariantMap HealthDataPipeline::getStatistics() const
{
    QVariantMap stats;
    stats["queuedItems"] = getQueuedItemCount();
    stats["processedCount"] = m_processedCount;
    stats["errorCount"] = m_errorCount;
    stats["running"] = m_running;
    return stats;
}

void HealthDataPipeline::setDefaultThresholds(const QVariantMap& thresholds)
{
    m_defaultThresholds = thresholds;
}

void HealthDataPipeline::setSubsystemThresholds(const QString& subsystemId,
                                                  const QVariantMap& thresholds)
{
    m_thresholds[subsystemId] = thresholds;
}

void HealthDataPipeline::processQueue()
{
    const int maxItemsPerTick = 10;
    int processed = 0;
    
    while (processed < maxItemsPerTick) {
        QueueItem item;
        
        {
            QMutexLocker locker(&m_queueMutex);
            if (m_dataQueue.isEmpty()) {
                break;
            }
            item = m_dataQueue.dequeue();
        }
        
        // Validate
        ValidationResult validation = validateData(item.subsystemId, item.data);
        
        if (!validation.valid) {
            m_errorCount++;
            emit validationError(item.subsystemId, validation.errorMessage);
            continue;
        }
        
        // Process
        ProcessingResult result = processData(item.subsystemId, validation.sanitizedData);
        
        // Emit results
        QVariantMap resultMap;
        resultMap["state"] = static_cast<int>(result.computedState);
        resultMap["healthScore"] = result.healthScore;
        resultMap["telemetry"] = result.processedTelemetry;
        resultMap["statusMessage"] = result.statusMessage;
        
        emit dataProcessed(item.subsystemId, resultMap);
        emit healthStateComputed(item.subsystemId, 
                                static_cast<int>(result.computedState),
                                result.healthScore);
        
        // Emit detected faults
        for (const auto& fault : result.detectedFaults) {
            emit faultDetected(item.subsystemId, fault.code);
        }
        
        m_processedCount++;
        processed++;
    }
    
    if (processed > 0) {
        emit statsChanged();
        emit queueChanged();
    }
}

bool HealthDataPipeline::validateDataTypes(const QVariantMap& data,
                                            const QVariantMap& schema) const
{
    Q_UNUSED(schema)
    
    // Basic type validation - ensure numeric values are actually numeric
    for (auto it = data.begin(); it != data.end(); ++it) {
        const QVariant& value = it.value();
        
        // Skip null values
        if (value.isNull()) {
            continue;
        }
        
        // Check for common invalid patterns
        if (value.type() == QVariant::String) {
            QString str = value.toString();
            if (str == "NaN" || str == "Infinity" || str == "-Infinity") {
                return false;
            }
        }
    }
    
    return true;
}

bool HealthDataPipeline::validateRanges(const QVariantMap& data,
                                         const QVariantMap& schema) const
{
    Q_UNUSED(schema)
    
    // Check for obviously invalid values
    for (auto it = data.begin(); it != data.end(); ++it) {
        const QVariant& value = it.value();
        
        if (value.canConvert<double>()) {
            double v = value.toDouble();
            
            // Check for invalid numeric values
            if (std::isnan(v) || std::isinf(v)) {
                return false;
            }
        }
    }
    
    return true;
}

bool HealthDataPipeline::validateRequired(const QVariantMap& data,
                                           const QVariantMap& schema) const
{
    if (!schema.contains("required")) {
        return true;
    }
    
    QVariantList required = schema["required"].toList();
    
    for (const QVariant& field : required) {
        if (!data.contains(field.toString())) {
            return false;
        }
    }
    
    return true;
}

QVariantMap HealthDataPipeline::sanitizeData(const QVariantMap& data) const
{
    QVariantMap sanitized;
    
    for (auto it = data.begin(); it != data.end(); ++it) {
        QVariant value = it.value();
        
        // Convert string numbers to actual numbers
        if (value.type() == QVariant::String) {
            bool ok;
            double d = value.toDouble(&ok);
            if (ok) {
                value = d;
            }
        }
        
        sanitized[it.key()] = value;
    }
    
    return sanitized;
}

HealthState HealthDataPipeline::computeHealthState(const QVariantMap& data,
                                                     const QVariantMap& thresholds) const
{
    bool hasCritical = false;
    bool hasWarning = false;
    
    for (auto it = data.begin(); it != data.end(); ++it) {
        if (!it.value().canConvert<double>()) {
            continue;
        }
        
        double value = it.value().toDouble();
        QString param = it.key();
        
        // Check parameter-specific thresholds
        if (thresholds.contains(param)) {
            QVariantMap paramThresholds = thresholds[param].toMap();
            
            if (paramThresholds.contains("criticalHigh") && 
                value > paramThresholds["criticalHigh"].toDouble()) {
                hasCritical = true;
            } else if (paramThresholds.contains("criticalLow") && 
                       value < paramThresholds["criticalLow"].toDouble()) {
                hasCritical = true;
            } else if (paramThresholds.contains("warningHigh") && 
                       value > paramThresholds["warningHigh"].toDouble()) {
                hasWarning = true;
            } else if (paramThresholds.contains("warningLow") && 
                       value < paramThresholds["warningLow"].toDouble()) {
                hasWarning = true;
            }
        }
    }
    
    if (hasCritical) {
        return HealthState::FAIL;
    } else if (hasWarning) {
        return HealthState::DEGRADED;
    }
    
    return HealthState::OK;
}

double HealthDataPipeline::computeHealthScore(const QVariantMap& data,
                                                const QVariantMap& thresholds) const
{
    double score = 100.0;
    int paramCount = 0;
    
    for (auto it = data.begin(); it != data.end(); ++it) {
        if (!it.value().canConvert<double>()) {
            continue;
        }
        
        double value = it.value().toDouble();
        QString param = it.key();
        
        if (thresholds.contains(param)) {
            QVariantMap paramThresholds = thresholds[param].toMap();
            paramCount++;
            
            // Calculate how close to threshold
            if (paramThresholds.contains("criticalHigh") && paramThresholds.contains("warningHigh")) {
                double critical = paramThresholds["criticalHigh"].toDouble();
                double warning = paramThresholds["warningHigh"].toDouble();
                
                if (value > critical) {
                    score -= 30;
                } else if (value > warning) {
                    double ratio = (value - warning) / (critical - warning);
                    score -= 10 + 20 * ratio;
                }
            }
            
            if (paramThresholds.contains("criticalLow") && paramThresholds.contains("warningLow")) {
                double critical = paramThresholds["criticalLow"].toDouble();
                double warning = paramThresholds["warningLow"].toDouble();
                
                if (value < critical) {
                    score -= 30;
                } else if (value < warning) {
                    double ratio = (warning - value) / (warning - critical);
                    score -= 10 + 20 * ratio;
                }
            }
        }
    }
    
    return qMax(0.0, qMin(100.0, score));
}

QList<FaultCode> HealthDataPipeline::detectFaults(const QString& subsystemId,
                                                    const QVariantMap& data,
                                                    const QVariantMap& thresholds) const
{
    QList<FaultCode> faults;
    
    for (auto it = data.begin(); it != data.end(); ++it) {
        if (!it.value().canConvert<double>()) {
            continue;
        }
        
        double value = it.value().toDouble();
        QString param = it.key();
        
        if (thresholds.contains(param)) {
            QVariantMap paramThresholds = thresholds[param].toMap();
            
            if (paramThresholds.contains("criticalHigh") && 
                value > paramThresholds["criticalHigh"].toDouble()) {
                FaultCode fault;
                fault.code = param.toUpper() + "-HIGH";
                fault.description = param + " exceeded critical threshold";
                fault.severity = FaultSeverity::CRITICAL;
                fault.subsystemId = subsystemId;
                fault.timestamp = QDateTime::currentDateTime();
                fault.active = true;
                faults.append(fault);
            }
            
            if (paramThresholds.contains("criticalLow") && 
                value < paramThresholds["criticalLow"].toDouble()) {
                FaultCode fault;
                fault.code = param.toUpper() + "-LOW";
                fault.description = param + " below critical threshold";
                fault.severity = FaultSeverity::CRITICAL;
                fault.subsystemId = subsystemId;
                fault.timestamp = QDateTime::currentDateTime();
                fault.active = true;
                faults.append(fault);
            }
        }
    }
    
    return faults;
}

} // namespace RadarRMP
