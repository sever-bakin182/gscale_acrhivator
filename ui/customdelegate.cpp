#include "customdelegate.h"

void CustomDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    if (m_selectedRows.contains(index.row())) {

        painter->save();

        painter->fillRect(option.rect, Qt::red);
        painter->setPen(Qt::blue);

        QString text = index.data(Qt::DisplayRole).toString();
        painter->drawText(option.rect, Qt::AlignCenter, text);

        painter->restore();
    }
};
