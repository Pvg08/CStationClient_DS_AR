#-------------------------------------------------
#
# Project created by QtCreator 2015-07-10T17:22:35
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CStation
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    server.cpp

HEADERS  += mainwindow.h \
    server.h

FORMS    += mainwindow.ui
