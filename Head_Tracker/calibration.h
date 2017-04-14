#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QDialog>
#include "typedef.h"
#include "readsensor.h"
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

namespace Ui {
class CalibDlg;
}

class Calib : public QDialog
{
    Q_OBJECT

private:
    Ui::CalibDlg *ui;
    ReadSensor *rs;
    quint16 numSamples;
    quint16 cnter;
    qint32 colIdx;

    cv::Mat_<double> accelMeas;
    cv::Mat_<double> accelReal;
    cv::Mat_<double> currAccelState;

    cv::Mat_<double> gyroMeas;
    cv::Mat_<double> gyroReal;
    cv::Mat_<double> currGyroState;

public:
    explicit Calib(ReadSensor *rs, QWidget *parent = 0);
    ~Calib();

private slots:
    void processData(SensorData data);
    void on_cancelBtn_clicked();
    void on_saveBtn_clicked();
    void on_zDownBtn_clicked();
    void on_zUpBtn_clicked();
    void on_yDownBtn_clicked();
    void on_yUpBtn_clicked();
    void on_xUpBtn_clicked();
    void on_xDownBtn_clicked();

private:
    void startRecording();
    void printMat(cv::Mat_<double> mat);

signals:
    void logSig(QString);
};

#endif // CALIBRATION_H
