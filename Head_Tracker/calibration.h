#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QObject>
#include "typedef.h"
#include "readsensor.h"

class Calib : public QObject
{
    Q_OBJECT

private:
    ReadSensor *rs;
    quint16 numSamples;
    quint16 cnter;
    QString calibFn;
    SensorData calibData;

public:
    explicit Calib(ReadSensor *rs, QString calibFn, QObject *parent = 0);

public slots:
    void start();

private slots:
    void processData(SensorData data);
    void saveCalib();

signals:
    void logSig(QString);
    void finished();
};

#endif // CALIBRATION_H
