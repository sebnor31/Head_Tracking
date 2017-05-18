#ifndef READSENSOR_H
#define READSENSOR_H

#include <QObject>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QPointer>
#include <QMutex>
#include "typedef.h"

class ReadTds : public QObject
{
    Q_OBJECT

private:
    QPointer<QSerialPort> sp;
    bool stopCond = false;
    QMutex mutex;
    const qint64 NUM_READ_BYTES = 28;

public:
    explicit ReadTds(QObject *parent = 0);
    ~ReadTds();

public slots:
    void start();
    void checkError(bool settingSuccess);
    void stop();

signals:
    void newDataSig(SensorData);
    void logSig(QString);
    void checkErrorSig(bool);
};

#endif // READSENSOR_H
