#include "core/SubsystemListModel.h"

namespace RadarRMP {

// ============================================================================
// SubsystemListModel
// ============================================================================

SubsystemListModel::SubsystemListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int SubsystemListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_subsystems.size();
}

QVariant SubsystemListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_subsystems.size()) {
        return QVariant();
    }
    
    RadarSubsystem* sub = m_subsystems.at(index.row());
    if (!sub) {
        return QVariant();
    }
    
    switch (role) {
        case IdRole:
            return sub->getId();
        case NameRole:
            return sub->getName();
        case TypeRole:
            return sub->getTypeName();
        case DescriptionRole:
            return sub->getDescription();
        case HealthStateRole:
            return sub->getHealthStateString();
        case HealthScoreRole:
            return sub->getHealthScore();
        case StatusMessageRole:
            return sub->getStatusMessage();
        case FaultCountRole:
            return sub->getFaultCount();
        case EnabledRole:
            return sub->isEnabled();
        case OnCanvasRole:
            return m_onCanvasIds.contains(sub->getId());
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> SubsystemListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[TypeRole] = "type";
    roles[DescriptionRole] = "description";
    roles[HealthStateRole] = "healthState";
    roles[HealthScoreRole] = "healthScore";
    roles[StatusMessageRole] = "statusMessage";
    roles[FaultCountRole] = "faultCount";
    roles[EnabledRole] = "enabled";
    roles[OnCanvasRole] = "onCanvas";
    return roles;
}

void SubsystemListModel::addSubsystem(RadarSubsystem* subsystem)
{
    if (!subsystem || m_indexMap.contains(subsystem->getId())) {
        return;
    }
    
    int index = m_subsystems.size();
    beginInsertRows(QModelIndex(), index, index);
    m_subsystems.append(subsystem);
    m_indexMap[subsystem->getId()] = index;
    endInsertRows();
    
    // Connect signals for updates - use Qt::QueuedConnection to batch updates
    connect(subsystem, &RadarSubsystem::healthChanged,
            this, &SubsystemListModel::onSubsystemDataChanged,
            Qt::QueuedConnection);
    connect(subsystem, &RadarSubsystem::faultsChanged,
            this, &SubsystemListModel::onSubsystemDataChanged,
            Qt::QueuedConnection);
}

void SubsystemListModel::removeSubsystem(const QString& id)
{
    if (!m_indexMap.contains(id)) {
        return;
    }
    
    int index = m_indexMap[id];
    
    beginRemoveRows(QModelIndex(), index, index);
    
    RadarSubsystem* sub = m_subsystems.takeAt(index);
    if (sub) {
        disconnect(sub, nullptr, this, nullptr);
    }
    
    m_indexMap.remove(id);
    m_onCanvasIds.remove(id);
    
    // Rebuild index map
    for (int i = index; i < m_subsystems.size(); ++i) {
        m_indexMap[m_subsystems[i]->getId()] = i;
    }
    
    endRemoveRows();
}

void SubsystemListModel::clear()
{
    if (m_subsystems.isEmpty()) {
        return;
    }
    
    beginResetModel();
    for (auto* sub : m_subsystems) {
        disconnect(sub, nullptr, this, nullptr);
    }
    m_subsystems.clear();
    m_indexMap.clear();
    m_onCanvasIds.clear();
    endResetModel();
}

RadarSubsystem* SubsystemListModel::getSubsystem(int index) const
{
    if (index < 0 || index >= m_subsystems.size()) {
        return nullptr;
    }
    return m_subsystems.at(index);
}

RadarSubsystem* SubsystemListModel::getSubsystemById(const QString& id) const
{
    if (!m_indexMap.contains(id)) {
        return nullptr;
    }
    return m_subsystems.at(m_indexMap[id]);
}

int SubsystemListModel::indexOf(const QString& id) const
{
    return m_indexMap.value(id, -1);
}

void SubsystemListModel::setOnCanvas(const QString& id, bool onCanvas)
{
    int idx = indexOf(id);
    if (idx < 0) {
        return;
    }
    
    bool wasOnCanvas = m_onCanvasIds.contains(id);
    if (wasOnCanvas == onCanvas) {
        return;
    }
    
    if (onCanvas) {
        m_onCanvasIds.insert(id);
    } else {
        m_onCanvasIds.remove(id);
    }
    
    QModelIndex modelIdx = index(idx);
    emit dataChanged(modelIdx, modelIdx, {OnCanvasRole});
}

bool SubsystemListModel::isOnCanvas(const QString& id) const
{
    return m_onCanvasIds.contains(id);
}

void SubsystemListModel::onSubsystemDataChanged()
{
    RadarSubsystem* sub = qobject_cast<RadarSubsystem*>(sender());
    if (sub) {
        refreshSubsystem(sub->getId());
    }
}

void SubsystemListModel::refreshSubsystem(const QString& id)
{
    int idx = indexOf(id);
    if (idx < 0) {
        return;
    }
    
    QModelIndex modelIdx = index(idx);
    emit dataChanged(modelIdx, modelIdx, {
        HealthStateRole, HealthScoreRole, StatusMessageRole, FaultCountRole
    });
}

void SubsystemListModel::refreshAll()
{
    if (m_subsystems.isEmpty()) {
        return;
    }
    
    emit dataChanged(index(0), index(m_subsystems.size() - 1));
}

// ============================================================================
// ActiveSubsystemModel
// ============================================================================

ActiveSubsystemModel::ActiveSubsystemModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ActiveSubsystemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_activeIds.size();
}

QVariant ActiveSubsystemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_activeIds.size()) {
        return QVariant();
    }
    
    if (!m_sourceModel) {
        return QVariant();
    }
    
    QString id = m_activeIds.at(index.row());
    RadarSubsystem* sub = m_sourceModel->getSubsystemById(id);
    if (!sub) {
        return QVariant();
    }
    
    switch (role) {
        case IdRole:
            return sub->getId();
        case NameRole:
            return sub->getName();
        case TypeRole:
            return sub->getTypeName();
        case HealthStateRole:
            return sub->getHealthStateString();
        case HealthScoreRole:
            return sub->getHealthScore();
        case FaultCountRole:
            return sub->getFaultCount();
        case EnabledRole:
            return sub->isEnabled();
        case SubsystemObjectRole:
            return QVariant::fromValue(sub);
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> ActiveSubsystemModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[TypeRole] = "type";
    roles[HealthStateRole] = "healthState";
    roles[HealthScoreRole] = "healthScore";
    roles[FaultCountRole] = "faultCount";
    roles[EnabledRole] = "enabled";
    roles[SubsystemObjectRole] = "subsystemObject";
    return roles;
}

void ActiveSubsystemModel::setSourceModel(SubsystemListModel* source)
{
    if (m_sourceModel) {
        disconnect(m_sourceModel, nullptr, this, nullptr);
    }
    
    m_sourceModel = source;
    
    if (m_sourceModel) {
        connect(m_sourceModel, &QAbstractListModel::dataChanged,
                this, &ActiveSubsystemModel::onSourceDataChanged);
    }
}

void ActiveSubsystemModel::addToCanvas(const QString& id)
{
    if (!m_sourceModel || m_activeIds.contains(id)) {
        return;
    }
    
    if (!m_sourceModel->getSubsystemById(id)) {
        return;
    }
    
    int index = m_activeIds.size();
    beginInsertRows(QModelIndex(), index, index);
    m_activeIds.append(id);
    m_sourceModel->setOnCanvas(id, true);
    endInsertRows();
    
    emit countChanged();
}

void ActiveSubsystemModel::removeFromCanvas(const QString& id)
{
    int index = m_activeIds.indexOf(id);
    if (index < 0) {
        return;
    }
    
    beginRemoveRows(QModelIndex(), index, index);
    m_activeIds.removeAt(index);
    if (m_sourceModel) {
        m_sourceModel->setOnCanvas(id, false);
    }
    endRemoveRows();
    
    emit countChanged();
}

void ActiveSubsystemModel::onSourceDataChanged()
{
    // Propagate data changes to active items
    if (!m_activeIds.isEmpty()) {
        emit dataChanged(index(0), index(m_activeIds.size() - 1));
    }
}

void ActiveSubsystemModel::refreshSubsystem(const QString& id)
{
    int idx = m_activeIds.indexOf(id);
    if (idx < 0) {
        return;
    }
    
    QModelIndex modelIdx = index(idx);
    emit dataChanged(modelIdx, modelIdx, {
        HealthStateRole, HealthScoreRole, FaultCountRole
    });
}

} // namespace RadarRMP
