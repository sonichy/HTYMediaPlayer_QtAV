#-------------------------------------------------
#
# Project created by QtCreator 2017-02-13T10:22:17
#
#-------------------------------------------------

QT       += core gui avwidgets av network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTYMPAV
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    controlpanel.cpp \
    dialogurl.cpp

HEADERS  += mainwindow.h \
    controlpanel.h \
    dialogurl.h

FORMS    += mainwindow.ui \
    controlpanel.ui \
    dialogurl.ui

RESOURCES += \
    filelist.qrc
