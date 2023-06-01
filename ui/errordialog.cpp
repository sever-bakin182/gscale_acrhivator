#include "errordialog.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ErrorDialog::ErrorDialog(const QString& msg, QWidget *parent)
    : QDialog(parent)
    , m_label(new QLabel(msg))
{
    QPushButton* button = new QPushButton("OK");

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(m_label);
    layout->addWidget(button);

    setLayout(layout);

    connect(button, &QPushButton::clicked, this, &QDialog::accept);

    setWindowTitle(tr("Error"));
}

void ErrorDialog::show(const QModelIndex& index, const QString& msg)
{
    m_index = index;
    if (msg != "") {
        m_label->setText(msg);
    }
    QDialog::show();
}

const QModelIndex ErrorDialog::getCurRow() const
{
    return m_index;
}



