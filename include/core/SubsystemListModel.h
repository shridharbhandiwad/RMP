#ifndef SUBSYSTEMLISTMODEL_H
#define SUBSYSTEMLISTMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QSet>
#include "RadarSubsystem.h"

namespace RadarRMP {

/**
 * @brief Proper QAbstractListModel for efficient QML binding
 * 
 * This model provides incremental updates to QML instead of
 * recreating the entire list on every change.
 */
class SubsystemListModel : public QAbstractListModel {
    Q_OBJECT
    
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TypeRole,
        DescriptionRole,
        HealthStateRole,
        HealthScoreRole,
        StatusMessageRole,
        FaultCountRole,
        EnabledRole,
        OnCanvasRole
    };
    
    explicit SubsystemListModel(QObject* parent = nullptr);
    
    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    // Model management
    void addSubsystem(RadarSubsystem* subsystem);
    void removeSubsystem(const QString& id);
    void clear();
    
    RadarSubsystem* getSubsystem(int index) const;
    RadarSubsystem* getSubsystemById(const QString& id) const;
    int indexOf(const QString& id) const;
    
    QList<RadarSubsystem*> subsystems() const { return m_subsystems; }
    
    // Canvas tracking
    void setOnCanvas(const QString& id, bool onCanvas);
    bool isOnCanvas(const QString& id) const;
    
public slots:
    void onSubsystemDataChanged();
    void refreshSubsystem(const QString& id);
    void refreshAll();
    
private:
    QList<RadarSubsystem*> m_subsystems;
    QHash<QString, int> m_indexMap;
    QSet<QString> m_onCanvasIds;
};

/**
 * @brief Model for active (on-canvas) subsystems only
 */
class ActiveSubsystemModel : public QAbstractListModel {
    Q_OBJECT
    
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TypeRole,
        HealthStateRole,
        HealthScoreRole,
        FaultCountRole,
        EnabledRole,
        SubsystemObjectRole
    };
    
    explicit ActiveSubsystemModel(QObject* parent = nullptr);
    
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    void setSourceModel(SubsystemListModel* source);
    void addToCanvas(const QString& id);
    void removeFromCanvas(const QString& id);
    
    int count() const { return m_activeIds.size(); }
    
public slots:
    void onSourceDataChanged();
    void refreshSubsystem(const QString& id);
    
signals:
    void countChanged();
    
private:
    SubsystemListModel* m_sourceModel = nullptr;
    QList<QString> m_activeIds;
};

} // namespace RadarRMP

#endif // SUBSYSTEMLISTMODEL_H
