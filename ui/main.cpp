#include "mainwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QStyle>

const int max_in_parameters = 2;



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString path_to_dir;
    if (argc == max_in_parameters) {
        path_to_dir = QString(argv[1]);
    }

    MainWindow window(path_to_dir);

    window.setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            window.size(),
            QApplication::desktop()->availableGeometry(&window)
        )
    );

    window.show();
    return a.exec();
}
