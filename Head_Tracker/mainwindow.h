#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "readsensor.h"
#include "qcustomplot.h"
#include "typedef.h"

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

private slots:
    void updatePlot(SensorData data);
    void logMsg(QString msg);

signals:
    void logSig(QString);
    void finished();
};

#endif // MAINWINDOW_H
