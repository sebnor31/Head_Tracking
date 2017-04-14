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
    qcustomplot.cpp \
    calibration.cpp

HEADERS  += mainwindow.h \
    readsensor.h \
    qcustomplot.h \
    typedef.h \
    calibration.h

FORMS    += mainwindow.ui \
    calibration.ui


# OPEN CV
INCLUDEPATH += "C:/dev/OpenCV/OpenCV_3_0/opencv/build/include"
LIBS        += -L"C:/dev/OpenCV/OpenCV_3_0/Build_Shared_64bit/lib/Release" \
                -lopencv_core300

