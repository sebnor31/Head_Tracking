#include "calibration.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

Calib::Calib(ReadSensor *rs, QString calibFn, QObject *parent) : QObject(parent)
{
    this->calibFn = calibFn;
    this->rs = rs;

    calibData.accelX = 0.0;
    calibData.accelY = 0.0;
    calibData.accelZ = 0.0;
    calibData.gyroX = 0.0;
    calibData.gyroY = 0.0;
    calibData.gyroZ = 0.0;
}

void Calib::start()
{
    numSamples = 100;
    cnter = numSamples;
    connect(rs, &ReadSensor::newDataSig, this, &Calib::processData);
}

void Calib::processData(SensorData data)
{
    cnter--;

    if (cnter <= 0){
        saveCalib();
        disconnect(rs, &ReadSensor::newDataSig, this, &Calib::processData);
        emit finished();
    }
    else{
        calibData.accelX += data.accelX;
        calibData.accelY += data.accelY;
        calibData.accelZ += data.accelZ;
        calibData.gyroX += data.gyroX;
        calibData.gyroY += data.gyroY;
        calibData.gyroZ += data.gyroZ;
    }
}

void Calib::saveCalib()
{
    calibData.accelX = calibData.accelX / numSamples;
    calibData.accelY = calibData.accelY / numSamples;
    calibData.accelZ = 1.0 + (calibData.accelZ / numSamples);

    calibData.gyroX = calibData.gyroX / numSamples;
    calibData.gyroY = calibData.gyroY / numSamples;
    calibData.gyroZ = calibData.gyroZ / numSamples;

    QFile calibFile(calibFn);
    QTextStream out(&calibFile);

    if (!calibFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit logSig("ERROR: Could not write calib params");
        return;
    }

    out << calibData.accelX << ","
        << calibData.accelY << ","
        << calibData.accelZ << "\n"
        << calibData.gyroX << ","
        << calibData.gyroY << ","
        << calibData.gyroZ << "\n";

    calibFile.close();

}
