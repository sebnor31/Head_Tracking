#include "readsensor.h"

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
}

void ReadSensor::checkError(bool settingSuccess)
{
    if (!settingSuccess){
        QString errMsg = QString("ERROR: Something went wrong with the receiver.\n\tDesc: %1\n\tCode: %2").arg(sp->errorString()).arg(sp->error());
        emit logSig(errMsg);
        sp->close();
    }
}

ReadSensor::~ReadSensor()
{
    if (sp != NULL){
        sp->close();
        emit logSig(QString("Serial Port Closed: %1").arg(!sp->isOpen()));
    }
}
