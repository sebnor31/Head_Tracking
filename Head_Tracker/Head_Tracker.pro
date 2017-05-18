#-------------------------------------------------
#
# Project created by QtCreator 2017-04-10T12:15:26
#
#-------------------------------------------------

QT       += core gui network widgets serialport printsupport

TARGET = Head_Tracker
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    calibration.cpp \
    readtds.cpp \
    readimupi.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    typedef.h \
    calibration.h \
    readtds.h \
    readimupi.h

FORMS    += mainwindow.ui \
    calibration.ui


# OPEN CV
INCLUDEPATH += "C:/dev/OpenCV/OpenCV_3_0/opencv/build/include"
LIBS        += -L"C:/dev/OpenCV/OpenCV_3_0/Build_Shared_64bit/lib/Release" \
                -lopencv_core300

