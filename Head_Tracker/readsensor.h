#ifndef READSENSOR_H
#define READSENSOR_H

#include <QObject>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QPointer>

class ReadSensor : public QObject
{
    Q_OBJECT

private:
    QPointer<QSerialPort> sp;

public:
    explicit ReadSensor(QObject *parent = 0);
    ~ReadSensor();

public slots:
    void start();
    void checkError(bool settingSuccess);

signals:
    void logSig(QString);
    void checkErrorSig(bool);
};

#endif // READSENSOR_H
