#-------------------------------------------------
#
# Project created by QtCreator 2017-04-10T12:15:26
#
#-------------------------------------------------

QT       += core gui widgets serialport printsupport

TARGET = Head_Tracker
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    readsensor.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    readsensor.h \
    qcustomplot.h \
    typedef.h

FORMS    += mainwindow.ui
