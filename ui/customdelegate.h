#ifndef CUSTOMDELEGATE_H
#define CUSTOMDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QSet>

class CustomDelegate: public QStyledItemDelegate
{
public:
    CustomDelegate(QSet<int>& selectedRows)
        : m_selectedRows(selectedRows)
    {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    QSet<int>& m_selectedRows;
};

#endif // CUSTOMDELEGATE_H
