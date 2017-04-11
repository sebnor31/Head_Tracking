#include "readsensor.h"
#include <QtMath>

ReadSensor::ReadSensor(QObject *parent) : QObject(parent)
{
}


void ReadSensor::start()
{
    // Connect to Mojo by identifying its COM Port
    bool portFound = false;
    QList<QSerialPortInfo> listOfPorts = QSerialPortInfo::availablePorts();

    foreach(QSerialPortInfo port, listOfPorts) {

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

    while (!stopCond) {

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
        bool dataAvail = sp->waitForReadyRead(1000);
        QByteArray data = sp->read(NUM_READ_BYTES);

        if (!dataAvail || (data.size() < NUM_READ_BYTES) ){
            QString errMsg = QString("WARNING: Could not read sensor data: Avail=%1 - Size=%2").arg(dataAvail).arg(data.size());
            emit logSig(errMsg);
            continue;
        }

        // accelerometer
        quint16 accelX = data.at(4)*qPow(2,8) + data.at(5);
        quint16 accelY = data.at(6)*qPow(2,8) + data.at(7);
        quint16 accelZ = data.at(8)*qPow(2,8) + data.at(9);

        // gyroscope
        quint16 gyroX = data.at(16)*qPow(2,8) + data.at(17);
        quint16 gyroY = data.at(18)*qPow(2,8) + data.at(19);
        quint16 gyroZ = data.at(20)*qPow(2,8) + data.at(21);

        emit logSig(QString("Accel [ %1 %2 %3 ] - Gyro [ %4 %5 %6 ]")
                    .arg(accelX).arg(accelY).arg(accelZ)
                    .arg(gyroX).arg(gyroY).arg(gyroZ));

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
