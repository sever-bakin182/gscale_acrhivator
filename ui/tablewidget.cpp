#include "tablewidget.h"

#include <QDir>

#include "tablemodel.h"

namespace {
QList<file_t> getFiles(const QString& path) {
    QDir dir(path);
    QStringList filters;
    filters << "*.bmp" << "*.png";

    QStringList ifiles = dir.entryList(filters, QDir::Files);
    QList<file_t> ofiles;
    for(auto& file: ifiles) {
        QFileInfo fileInfo(dir, file);
        ofiles.append(file_t(file, fileInfo.size()));
    }

    ifiles.clear();
    filters.clear();
    filters << "*.barch";
    ifiles = dir.entryList(filters, QDir::Files);
    for(auto& file: ifiles) {
        QFileInfo fileInfo(dir, file);
        ofiles.append(file_t(file, fileInfo.size()));
    }

    return ofiles;
}
}

TableWidget::TableWidget(const QString& path, QWidget *parent)
    : QListView(parent)
{
    QList<TableModel::raw_t> files = getFiles(path);
    m_model = new TableModel(files, this);
}

