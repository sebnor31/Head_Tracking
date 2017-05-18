#ifndef TYPEDEF_H
#define TYPEDEF_H

#include <QObject>

struct SensorData{
    double accelX;
    double accelY;
    double accelZ;
    double gyroX;
    double gyroY;
    double gyroZ;
    double magX;
    double magY;
    double magZ;
    double time;
    quint8 pktCnter;
};
const int dontcare = qRegisterMetaType<SensorData>("SensorData");

#endif // TYPEDEF_H
