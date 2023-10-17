#-------------------------------------------------
#
# Project created by QtCreator 2018-07-13T14:20:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = RealTimeGraph
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += /usr/local/include/opencv4
LIBS += `pkg-config --cflags --libs opencv4`



# -L represents for Directory

# -l represents for file
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
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
