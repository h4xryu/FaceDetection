QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = RealTimeGraph
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += /usr/local/include/opencv4
LIBS += `pkg-config --cflags --libs opencv4`

SOURCES += \
    Thread.cpp \
        main.cpp \
        mainwindow.cpp \
    qcustomplot.cpp

HEADERS += \
        mainwindow.h \
        qcustomplot.h

FORMS += \
        mainwindow.ui

DISTFILES += \
    ../../../test/video_test/object_size \
    object_size.txt \
    vtest.avi
