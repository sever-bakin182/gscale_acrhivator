#include "tablemodel.h"

namespace {
    const int MAX_SIZE_COLUMN = 3;
}

TableModel::TableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

TableModel::TableModel(const QList<file_t>& listOfFiles, QObject *parent)
    : QAbstractTableModel(parent)
    , m_listOfFiles(listOfFiles)
{}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return tr("Status");
            case 1:
                return tr("Name");
            case 2:
                return tr("Size");
            default:
                break;
        }
    }
    return QVariant();
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_listOfFiles.size();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : MAX_SIZE_COLUMN;
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_listOfFiles.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &fileIinfo = m_listOfFiles.at(index.row());

        switch (index.column()) {
            case 0:
                return getStatusStr(fileIinfo.status);
            case 1:
                return fileIinfo.name;
            case 2:
                return fileIinfo.size;
            default:
                break;
        }
    }

    return QVariant();
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        const int row = index.row();
        auto fileInfo = m_listOfFiles.value(row);

        switch (index.column()) {
            case 0:
                fileInfo.status = static_cast<Status>(value.toInt());
                break;
            case 1:
                fileInfo.name = value.toString();
                break;
            case 2:
                fileInfo.size = value.toULongLong();
                break;
            default:
                return false;
        }
        m_listOfFiles.replace(row, fileInfo);
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

        return true;
    }

    return false;
}

bool TableModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        m_listOfFiles.removeAt(position);

    endRemoveRows();
    return true;
}

QString TableModel::getStatusStr(TableModel::Status status) const
{
        switch (status) {
            case Status::None:
                return tr("None");
            case Status::Compression:
                return tr("Compression");
            case Status::Decompression:
                return tr("Decompression");
            case Status::Done:
                return tr("Done");
            default:
                return QString();
        }
}

bool TableModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        m_listOfFiles.insert(position, { QString(), 0 });

    endInsertRows();
    return true;
}
