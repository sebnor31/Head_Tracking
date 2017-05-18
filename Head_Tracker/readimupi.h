#ifndef READIMUPI_H
#define READIMUPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include "typedef.h"

class readImuPi : public QObject
{
    Q_OBJECT

private:
    // Timing
    bool firstData = true;
    qint64 basetime;
    QTimer *timer;

    // Network
    QNetworkAccessManager *manager;
    QNetworkRequest request;

public:
    explicit readImuPi(QObject *parent = 0);
    ~readImuPi();

public slots:
    void start();
    void stop();

private slots:
    void readData();
    void logNetError(QNetworkReply::NetworkError err);
    void dataReceive(QNetworkReply *reply);

signals:
    void newDataSig(SensorData);
    void logSig(QString);
};

#endif // READIMUPI_H
