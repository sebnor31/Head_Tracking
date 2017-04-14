#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "readsensor.h"
#include "qcustomplot.h"
#include "typedef.h"
#include <opencv2/core/matx.hpp>
#include <opencv2/core/mat.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

/*****************
 * Variables
 * **************/
private:
    Ui::MainWindow *ui;
    QThread *rsTh;
    ReadSensor *rs;
    cv::Matx33d accelGain;
    cv::Matx31d accelOffset;
    cv::Matx33d gyroGain;
    cv::Matx31d gyroOffset;

    struct DataPlot {
        QCPGraph *graph;
        QCPAxisRect *axis;
        QCPRange *timeRange;
        QCPRange *YRange;
    };

    DataPlot accelPlot[3];
    DataPlot gyroPlot [3];


/*****************
 * Methods
 * **************/
public:
    explicit MainWindow(QWidget *parent = 0);
    void closeEvent(QCloseEvent * event);
    ~MainWindow();

private:
    void setPlot();
    QString printMat(cv::Mat mat);

private slots:
    void processData(SensorData data);
    void updatePlot(SensorData data);
    void logMsg(QString msg);
    void on_calibBtn_clicked();
    void on_deviceIdCombo_currentIndexChanged(int index);

signals:
    void logSig(QString);
    void procDataSig(SensorData);
    void finished();
};

#endif // MAINWINDOW_H
