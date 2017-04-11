#ifndef READSENSOR_H
#define READSENSOR_H

#include <QObject>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QPointer>
#include <QMutex>

class ReadSensor : public QObject
{
    Q_OBJECT

private:
    QPointer<QSerialPort> sp;
    bool stopCond = false;
    QMutex mutex;
    const qint64 NUM_READ_BYTES = 28;

public:
    explicit ReadSensor(QObject *parent = 0);
    ~ReadSensor();

public slots:
    void start();
    void checkError(bool settingSuccess);
    void stop();

signals:
    void logSig(QString);
    void checkErrorSig(bool);
};

#endif // READSENSOR_H
