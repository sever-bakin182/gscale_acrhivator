#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QListView>

class TableModel;

class TableWidget : public QListView
{
    Q_OBJECT

public:
    explicit TableWidget(const QString& path, QWidget *parent = nullptr);

private:
    TableModel* m_model;
};

#endif // TABLEWIDGET_H
