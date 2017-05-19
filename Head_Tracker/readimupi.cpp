#include "readimupi.h"
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QJsonArray>

readImuPi::readImuPi(QObject *parent) : QObject(parent)
{

}

void readImuPi::start()
{
    QUrl rpi("http://169.254.0.2:8800/");
    emit logSig(QString("Connecting to Rpi at: %1").arg(rpi.toString()));

    manager = new QNetworkAccessManager(this);
    request.setUrl(rpi);
    connect(manager, &QNetworkAccessManager::finished, this, &readImuPi::dataReceive);

    timer = new QTimer(this);
    timer->setInterval(20);
    connect(timer, &QTimer::timeout, this, &readImuPi::readData);
    timer->start();

}

void readImuPi::readData()
{
    manager->get(request);
}

void readImuPi::dataReceive(QNetworkReply *reply)
{

    if (reply->error() != QNetworkReply::NoError){
        logNetError(reply->error());
    }
    else{
        QByteArray resp = reply->readAll();
        emit logSig(QString(resp));

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(resp, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            emit logSig(parseError.errorString());
            return;
        }

        QJsonObject jsonObj = doc.object();
        QJsonArray magArray = jsonObj.value("mag").toArray();
        QJsonArray gyroArray = jsonObj.value("gyro").toArray();
        QJsonArray accelArray = jsonObj.value("accel").toArray();

        // accelerometer
        int rawAccelX = accelArray.at(0).toInt();
        int rawAccelY = accelArray.at(1).toInt();
        int rawAccelZ = accelArray.at(2).toInt();

        double accelX = 2.0 * (rawAccelX) / 32768.0;
        double accelY = 2.0 * (rawAccelY) / 32768.0;
        double accelZ = 2.0 * (rawAccelZ) / 32768.0;
//        double accelX = rawAccelX;
//        double accelY = rawAccelY;
//        double accelZ = rawAccelZ;

        // gyroscope
        int rawGyroX = gyroArray.at(0).toInt();
        int rawGyroY = gyroArray.at(1).toInt();
        int rawGyroZ = gyroArray.at(2).toInt();

        double gyroX = 245.0 * (rawGyroX) / 32768.0;
        double gyroY = 245.0 * (rawGyroY) / 32768.0;
        double gyroZ = 245.0 * (rawGyroZ) / 32768.0;
//        double gyroX = rawGyroX;
//        double gyroY = rawGyroY;
//        double gyroZ = rawGyroZ;

        // magnetometer
        int rawMagX = magArray.at(0).toInt();
        int rawMagY = magArray.at(1).toInt();
        int rawMagZ = magArray.at(2).toInt();

        double magX = 4.0 * (rawMagX) / 32768.0;
        double magY = 4.0 * (rawMagY) / 32768.0;
        double magZ = 4.0 * (rawMagZ) / 32768.0;
//        double magX = rawMagX;
//        double magY = rawMagY;
//        double magZ = rawMagZ;

        // Display normalized values
        if (firstData) {
            basetime = QDateTime::currentMSecsSinceEpoch();
            firstData = false;
        }

        SensorData sensorData;
        sensorData.accelX = accelX;
        sensorData.accelY = accelY;
        sensorData.accelZ = accelZ;
        sensorData.gyroX = gyroX;
        sensorData.gyroY = gyroY;
        sensorData.gyroZ = gyroZ;
        sensorData.magX = magX;
        sensorData.magY = magY;
        sensorData.magZ = magZ;
        sensorData.pktCnter = 0;
        sensorData.time = (QDateTime::currentMSecsSinceEpoch() - basetime) / 1000.0;
        emit newDataSig(sensorData);
    }

    reply->deleteLater();
}

void readImuPi::logNetError(QNetworkReply::NetworkError err)
{
    QString msg = QString("Network Error: %1").arg(err);
    emit logSig(msg);
}

void readImuPi::stop()
{
    /* **********************************************
     * TODO
     * Send a request to server to turn off raspberry pi
     * *********************************************/
    timer->stop();
    delete timer;
}

readImuPi::~readImuPi()
{
}
