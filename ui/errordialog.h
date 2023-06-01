#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include <QModelIndex>

class QLabel;

class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(const QString& msg, QWidget *parent = nullptr);

    void show(const QModelIndex& index_row, const QString& message = "");
    const QModelIndex getCurRow() const;

private:
    QModelIndex m_index;
    QLabel* m_label;
};

#endif // ERRORDIALOG_H
