greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++14
TEMPLATE = app
TARGET = archivergs

INCLUDEPATH += ../lib/gscalelib
LIBS += -L../lib/gscalelib/build -lgscale

SOURCES += \
    customdelegate.cpp \
    errordialog.cpp \
    main.cpp \
    mainwindow.cpp \
    tablemodel.cpp

HEADERS += \
    customdelegate.h \
    errordialog.h \
    mainwindow.h \
    tablemodel.h
    
DESTDIR = build
OBJECTS_DIR = build/objects
MOC_DIR = build/moc
