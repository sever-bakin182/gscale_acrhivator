#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>

class TableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum class Status {
        None,
        Compression,
        Decompression,
        Done
    };

    struct file_t {
        QString name;
        qint64 size;
        Status status;
    };

    explicit TableModel(QObject *parent = nullptr);
    TableModel(const QList<file_t>& listOfFiles, QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

private:
    QList<file_t> m_listOfFiles;

    QString getStatusStr(Status status) const;
};

#endif // TABLEMODEL_H
