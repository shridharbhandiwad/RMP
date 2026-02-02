#ifndef TELEMETRYDATA_H
#define TELEMETRYDATA_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QDateTime>

namespace RadarRMP {

/**
 * @brief Telemetry parameter metadata
 */
struct TelemetryParameter {
    QString name;           // Parameter identifier
    QString displayName;    // Human-readable name
    QString unit;           // Unit of measurement
    QVariant value;         // Current value
    QVariant nominal;       // Nominal/expected value
    QVariant minValue;      // Minimum valid value
    QVariant maxValue;      // Maximum valid value
    QVariant warningLow;    // Low warning threshold
    QVariant warningHigh;   // High warning threshold
    QVariant criticalLow;   // Low critical threshold
    QVariant criticalHigh;  // High critical threshold
    QDateTime timestamp;    // Last update time
    bool isValid;           // Data validity flag
    
    TelemetryParameter() : isValid(false) {}
    
    TelemetryParameter(const QString& n, const QString& dn, const QString& u)
        : name(n), displayName(dn), unit(u),
          timestamp(QDateTime::currentDateTime()), isValid(true) {}
    
    QVariantMap toVariantMap() const {
        QVariantMap map;
        map["name"] = name;
        map["displayName"] = displayName;
        map["unit"] = unit;
        map["value"] = value;
        map["nominal"] = nominal;
        map["minValue"] = minValue;
        map["maxValue"] = maxValue;
        map["warningLow"] = warningLow;
        map["warningHigh"] = warningHigh;
        map["criticalLow"] = criticalLow;
        map["criticalHigh"] = criticalHigh;
        map["timestamp"] = timestamp;
        map["isValid"] = isValid;
        return map;
    }
};

/**
 * @brief Container for telemetry data with validation and thresholds
 */
class TelemetryData : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap data READ getData NOTIFY dataChanged)
    Q_PROPERTY(QDateTime lastUpdate READ getLastUpdate NOTIFY dataChanged)
    
public:
    explicit TelemetryData(QObject* parent = nullptr);
    ~TelemetryData() override = default;
    
    // Parameter management
    void addParameter(const TelemetryParameter& param);
    void removeParameter(const QString& name);
    bool hasParameter(const QString& name) const;
    TelemetryParameter getParameter(const QString& name) const;
    QStringList getParameterNames() const;
    
    // Value access
    QVariant getValue(const QString& name) const;
    void setValue(const QString& name, const QVariant& value);
    void setValues(const QVariantMap& values);
    
    // Threshold checking
    bool isWithinLimits(const QString& name) const;
    bool isWarning(const QString& name) const;
    bool isCritical(const QString& name) const;
    
    // Bulk access
    QVariantMap getData() const;
    QVariantMap getMetadata() const;
    QDateTime getLastUpdate() const;
    
    // Validation
    void validate();
    bool isAllValid() const;
    QStringList getInvalidParameters() const;
    
signals:
    void dataChanged();
    void parameterChanged(const QString& name, const QVariant& value);
    void thresholdExceeded(const QString& name, const QString& type);
    void validityChanged(const QString& name, bool valid);
    
private:
    QMap<QString, TelemetryParameter> m_parameters;
    QDateTime m_lastUpdate;
};

} // namespace RadarRMP

#endif // TELEMETRYDATA_H
