#include "readsensor.h"
#include <QtMath>
#include <QDateTime>
#include <QThread>

ReadSensor::ReadSensor(QObject *parent) : QObject(parent)
{
}


void ReadSensor::start()
{
    // Connect to Mojo by identifying its COM Port
    bool portFound = false;
    QList<QSerialPortInfo> listOfPorts = QSerialPortInfo::availablePorts();

    foreach(QSerialPortInfo port, listOfPorts) {
        emit logSig(QString("PID: %1 - Manuf: %2").arg(port.productIdentifier()).arg(port.manufacturer()));
        if (port.productIdentifier() == 24577) {
            sp = new QSerialPort(port, this);
            portFound = true;
            break;
        }
    }

    // Set Serial Port
    if (!portFound) {
        emit logSig(QString("ERROR: Could not find the receiver. Make sure it is connected"));
        return;
    }

    emit logSig(QString("SUCCESS: Receiver found in %1").arg(sp->portName()));
    checkErrorSig(sp->open(QIODevice::ReadWrite));
    checkErrorSig(sp->setBaudRate(921600));
    checkErrorSig(sp->setDataBits(QSerialPort::Data8));
    checkErrorSig(sp->setParity(QSerialPort::NoParity));
    checkErrorSig(sp->setStopBits(QSerialPort::OneStop));
    sp->setReadBufferSize(1);


    // Perform Handshake
    bool connected = false;
    QByteArray handshake;
    handshake.resize(6);
    handshake[0] = static_cast<quint8>(0xCC);    //204
    handshake[1] = static_cast<quint8>(0xCC);    //204
    handshake[2] = static_cast<quint8>(0x03);    //3
    handshake[3] = static_cast<quint8>(0xE8);    //232
    handshake[4] = static_cast<quint8>(0x01);    //1
    handshake[5] = static_cast<quint8>(0x00);    //0

    emit logSig("WARNING: Please turn ON the Headset");

    while (!connected && !stopCond){

        qint64 numBytes = sp->write(handshake);
        bool dataWritten = sp->waitForBytesWritten(100);

        if(numBytes == -1 || !dataWritten) {
            QString errMsg = QString("ERROR: Could not write to receiver.\n\tDesc: %1\n\tCode: %2").arg(sp->errorString()).arg(sp->error());
            emit logSig(errMsg);
            continue;
        }

        else if (numBytes != 6) {
            QString errMsg = QString("ERROR: Could not write proper #bytes to receiver: %1 instead of 6").arg(numBytes);
            emit logSig(errMsg);
            continue;
        }

        bool readAvail = sp->waitForReadyRead(1000);

        if (!readAvail){
            emit logSig("TIMEOUT: No data to read");
            continue;
        }

        QByteArray data = sp->read(1);
        if (data.isEmpty()) {
            emit logSig("WARNING: Empty Read");
            continue;

        } else {
            emit logSig("SUCCESS: TDS is connected");
            connected = true;
        }
    }

    // Read sensor data
    sp->clear();    // Reset internal read buffer
    sp->setReadBufferSize(NUM_READ_BYTES);
    QByteArray fetchCmd;
    fetchCmd.resize(6);
    fetchCmd[0] = static_cast<quint8>(0xAA);
    fetchCmd[1] = static_cast<quint8>(0xAA);
    fetchCmd[2] = static_cast<quint8>(0x0F);
    fetchCmd[3] = static_cast<quint8>(0xFF);
    fetchCmd[4] = static_cast<quint8>(0x0F);
    fetchCmd[5] = static_cast<quint8>(0xFF);

    qint64 basetime = QDateTime::currentMSecsSinceEpoch();

    while (!stopCond) {

        QThread::msleep(20);    // Give time for a new sample to be received

        // Send Fetch Command
        qint64 numBytes = sp->write(fetchCmd);
        bool dataWritten = sp->waitForBytesWritten(100);

        if(numBytes == -1 || !dataWritten) {
            QString errMsg = QString("ERROR: Could not write to receiver.\n\tDesc: %1\n\tCode: %2").arg(sp->errorString()).arg(sp->error());
            emit logSig(errMsg);
            continue;
        }

        else if (numBytes != 6) {
            QString errMsg = QString("ERROR: Could not write proper #bytes to receiver: %1 instead of 6").arg(numBytes);
            emit logSig(errMsg);
            continue;
        }

        // Read sensor values
        bool dataAvail = true;
        QByteArray data = sp->read(NUM_READ_BYTES);

        if (!dataAvail || (data.size() < NUM_READ_BYTES) ){
            sp->clear();
            QString errMsg = QString("WARNING: Could not read sensor data: Avail=%1 - Size=%2").arg(dataAvail).arg(data.size());
            emit logSig(errMsg);

            continue;
        }

        // Headers
        quint8 pktCnter = data.at(1);   // Packet Counter

        // accelerometer
        quint16 rawAccelX = data.at(4)*qPow(2,8) + data.at(5);
        quint16 rawAccelY = data.at(6)*qPow(2,8) + data.at(7);
        quint16 rawAccelZ = data.at(8)*qPow(2,8) + data.at(9);

        double accelX = 2.0 * (rawAccelX - 32768) / 32768.0;
        double accelY = 2.0 * (rawAccelY - 32768) / 32768.0;
        double accelZ = 2.0 * (rawAccelZ - 32768) / 32768.0;

        // gyroscope
        quint16 rawGyroX = data.at(16)*qPow(2,8) + data.at(17);
        quint16 rawGyroY = data.at(18)*qPow(2,8) + data.at(19);
        quint16 rawGyroZ = data.at(20)*qPow(2,8) + data.at(21);

        double gyroX = 250.0 * (rawGyroX - 32768) / 32768.0;
        double gyroY = 250.0 * (rawGyroY - 32768) / 32768.0;
        double gyroZ = 250.0 * (rawGyroZ - 32768) / 32768.0;

        // Display normalized values
        SensorData sensorData;
        sensorData.accelX = accelX;
        sensorData.accelY = accelY;
        sensorData.accelZ = accelZ;
        sensorData.gyroX = gyroX;
        sensorData.gyroY = gyroY;
        sensorData.gyroZ = gyroZ;
        sensorData.pktCnter = pktCnter;
        sensorData.time = (QDateTime::currentMSecsSinceEpoch() - basetime) / 1000.0;
        emit newDataSig(sensorData);

        emit logSig(QString("Pkt %7 (%8): Accel [ %1 %2 %3 ] - Gyro [ %4 %5 %6 ]")
                    .arg(accelX).arg(accelY).arg(accelZ)
                    .arg(gyroX).arg(gyroY).arg(gyroZ)
                    .arg(pktCnter).arg(sensorData.time));
    }
}

void ReadSensor::checkError(bool settingSuccess)
{
    if (!settingSuccess){
        QString errMsg = QString("ERROR: Something went wrong with the receiver.\n\tDesc: %1\n\tCode: %2").arg(sp->errorString()).arg(sp->error());
        emit logSig(errMsg);
        sp->close();
    }
}

void ReadSensor::stop()
{
    mutex.lock();
    stopCond = true;
    mutex.unlock();
}

ReadSensor::~ReadSensor()
{
    if (sp != NULL){
        sp->close();
        emit logSig(QString("Serial Port Closed: %1").arg(!sp->isOpen()));
    }
}
