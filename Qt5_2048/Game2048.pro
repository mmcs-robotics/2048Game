#-------------------------------------------------
#
# Project created by QtCreator 2016-05-11T23:34:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TextEditor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tiles.cpp \
    state.cpp \
    threads.cpp

HEADERS  += mainwindow.h \
    threads.h \
    tiles.h \
    state.h

FORMS    += mainwindow.ui

RESOURCES += \
    myres.qrc
