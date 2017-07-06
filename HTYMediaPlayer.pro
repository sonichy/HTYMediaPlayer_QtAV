#-------------------------------------------------
#
# Project created by QtCreator 2017-02-13T10:22:17
#
#-------------------------------------------------

QT       += core gui avwidgets av

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTYMediaPlayer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    controlpanel.cpp

HEADERS  += mainwindow.h \
    controlpanel.h

FORMS    += mainwindow.ui \
    controlpanel.ui

RESOURCES += \
    filelist.qrc
