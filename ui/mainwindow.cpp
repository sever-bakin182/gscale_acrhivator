#include "mainwindow.h"

#include "tablemodel.h"
#include "errordialog.h"
#include "customdelegate.h"
#include "errordialog.h"

#include <QDir>
#include <QTableView>

#include <memory>
#include <thread>

#include <archiver_gscale.h>

using namespace gscale;

namespace {
QList<TableModel::file_t> getFiles(const QString& path) {
    QDir dir(path);
    QStringList filters;
    filters << "*.bmp" << "*.png";

    QStringList ifiles = dir.entryList(filters, QDir::Files);
    QList<TableModel::file_t> ofiles;
    for(auto& file: ifiles) {
        QFileInfo fileInfo(dir, file);
        TableModel::file_t f;
        f.status = TableModel::Status::None;
        f.name = file;
        f.size = fileInfo.size();
        ofiles.append(f);
    }

    ifiles.clear();
    filters.clear();
    filters << "*.barch";
    ifiles = dir.entryList(filters, QDir::Files);
    for(auto& file: ifiles) {
        QFileInfo fileInfo(dir, file);
        TableModel::file_t f;
        f.name = file;
        f.size = fileInfo.size();
        ofiles.append(f);
    }

    return ofiles;
}

QString getHeadOfName(const QString& v) {
    QFileInfo fileInfo(v);
    return fileInfo.baseName();
}
}

MainWindow::MainWindow(const QString& path, QWidget *parent)
    : QMainWindow(parent)
    , m_path(path)
    , m_model(new TableModel(getFiles(m_path)))
    , m_view(new QTableView)
    , m_error(new ErrorDialog("Error", m_view))
    , m_deleg(new CustomDelegate(m_selectedRows))
{
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    //m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_view->setModel(m_model);
    m_view->setItemDelegate(m_deleg);
    m_view->resizeColumnsToContents();
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    initConnects();

    setCentralWidget(m_view);
}

MainWindow::~MainWindow()
{
    disconnect(m_view, &QTableView::clicked, this, &MainWindow::handleClicked);
    disconnect(m_error, &QDialog::accepted, this, &MainWindow::setRedRow);
}

void MainWindow::initConnects() {
    connect(m_view, &QTableView::clicked, this, &MainWindow::handleClicked);
    connect(m_error, &QDialog::accepted, this, &MainWindow::setRedRow);
}

bool MainWindow::deleteFile(const QString &name)
{
    return true;//come back => QFile::remove(getAbsolutePathToFile(name));
}

bool MainWindow::writeToFile(const unsigned char* idata, const QString &name, const std::size_t size)
{
    bool result = false;
    QString absolute_path = getAbsolutePathToFile(name);
    QFile file(absolute_path);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream stream(&file);
        int write_size = stream.writeRawData(reinterpret_cast<const char*>(idata), static_cast<int>(size));
        result = (write_size == static_cast<int>(size));
        file.close();
    }

    return result;
}

bool MainWindow::readFromFile(unsigned char* odata, const QString &name, const std::size_t size)
{
    bool result = false;
    QString absolute_path = getAbsolutePathToFile(name);
    QFile file(absolute_path);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream stream(&file);
        int read_size = stream.readRawData(reinterpret_cast<char*>(odata), static_cast<int>(size));
        result = (read_size == static_cast<int>(size));
        file.close();
    }

    return result;
}

void MainWindow::updateRow(const QModelIndex &index, const QString &name, const size_t size)
{
    m_model->setData(m_model->index(index.row(), 1), name);
    m_model->setData(m_model->index(index.row(), 2), static_cast<const qulonglong>(size));
}

void MainWindow::updateRowStatus(const QModelIndex &index, TableModel::Status status)
{
    m_model->setData(m_model->index(index.row(), 0), static_cast<int>(status));
}

QString MainWindow::getAbsolutePathToFile(const QString &name)
{
    return (QDir(m_path).absolutePath() + "/" + name);
}

void MainWindow::setRedRow()
{
    QModelIndex index = m_error->getCurRow();
    m_selectedRows.insert(index.row());
    m_view->update(index);
}

void MainWindow::handleClicked(const QModelIndex &index) {
    if (index.isValid()) {
        /*start thread*/
        std::thread([this, &index](){
            QString value;
            std::size_t fileSize = 0;
            //get name and size of file
            if (index.column() == 0) {
                QModelIndex firstColumnIndex = index.sibling(index.row(), 1);
                value = m_model->data(firstColumnIndex).toString();
                QModelIndex secondColumnIndex = index.sibling(index.row(), 2);
                fileSize = m_model->data(secondColumnIndex).toUInt();
            } else if (index.column() == 1){
                value = index.data().toString();
                QModelIndex secondColumnIndex = index.sibling(index.row(), 2);
                fileSize = m_model->data(secondColumnIndex).toUInt();
            } else if (index.column() == 2) {
                fileSize = index.data().toUInt();
                QModelIndex firstColumnIndex = index.sibling(index.row(), 1);
                value = m_model->data(firstColumnIndex).toString();
            }

            //get data of file
            std::unique_ptr<unsigned char[]> ibuffer(new unsigned char[fileSize]);

            if (!readFromFile(ibuffer.get(), value, fileSize)) {
                m_error->show(index, "Cannot read file! =>" + value);
                return;
            }

            RawImageData rdata;
            InOutData iodata;
            QString newNameOfFile;
            std::size_t lenNewFile = 0;

            /*what type of file is*/
            if (value.endsWith("barch")) {
                /*extract height and width from name file*/
                iodata.data = ibuffer.get();
                iodata.size = fileSize;

                updateRowStatus(index, TableModel::Status::Decompression);
                lenNewFile = decompress(rdata, iodata);
                if (lenNewFile == 0) {
                    m_error->show(index, "Cannot decompress file!");
                    return;
                }

                //delete old file create new
                if (!deleteFile(value)) {
                    m_error->show(index, "Cannot delete file! =>" + value);
                    delete [] rdata.data;
                    return;
                }

                newNameOfFile = getHeadOfName(value) + "unpacked.bmp";
                if (!writeToFile(rdata.data, newNameOfFile, lenNewFile)) {
                    m_error->show(index, "Cannot write to file! =>" + newNameOfFile);
                    delete [] rdata.data;
                    return;
                }
                delete [] rdata.data;
                updateRowStatus(index, TableModel::Status::Done);
            } else if (value.endsWith("bmp")) { // check bmp
                /*extract height and width from file*/
                QImage image(getAbsolutePathToFile(value));
                if (image.isNull()) {
                    m_error->show(index, "File isn't an image!");
                    return;
                }

                rdata.width = image.width();
                rdata.height = image.height();
                rdata.data = ibuffer.get();
                updateRowStatus(index, TableModel::Status::Compression);
                lenNewFile = compress(rdata, fileSize, iodata);
                if (lenNewFile == 0) {
                    m_error->show(index, "Cannot compress file!");
                    return;
                }

                if (!deleteFile(value)) {
                    m_error->show(index, "Cannot delete file! =>" + value);
                    freeOutputData(iodata);
                    return;
                }

                newNameOfFile = getHeadOfName(value) + "packed.barch";
                if (!writeToFile(iodata.data, newNameOfFile,lenNewFile)) {
                    m_error->show(index, "Cannot create file! =>" + newNameOfFile);
                    freeOutputData(iodata);
                    return;
                }
                freeOutputData(iodata);
                updateRowStatus(index, TableModel::Status::Done);
            } else {
                m_error->show(index, "Not rules for current file! =>" + value);
                return;
            }
            updateRow(index, newNameOfFile, lenNewFile);
        }).detach();
    }
}
