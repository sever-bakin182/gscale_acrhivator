#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSet>

#include "tablemodel.h"

class QTableView;
class ErrorDialog;
class CustomDelegate;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString& path, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void setStatus(const QModelIndex&, TableModel::Status);
private slots:
    void handleClicked(const QModelIndex &index);
    void setRedRow();

private:
    void initConnects();

    bool deleteFile(const QString& name);
    bool writeToFile(const unsigned char* odata, const QString& name, const std::size_t);
    bool readFromFile(unsigned char* idata, const QString& name, const std::size_t);
    void updateRow(const QModelIndex &index, const QString& name, const size_t size);
    void updateRowStatus(const QModelIndex &index, TableModel::Status);
    QString getAbsolutePathToFile(const QString& name);

    QString m_path;

    TableModel* m_model;
    QTableView* m_view;
    ErrorDialog* m_error;
    CustomDelegate* m_deleg;

    QSet<int> m_selectedRows;
};
#endif // MAINWINDOW_H
