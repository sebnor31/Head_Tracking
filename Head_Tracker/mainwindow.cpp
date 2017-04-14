#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calibration.h"

#include <QDebug>
#include <QMessageBox>
#include <QDir>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    rsTh = new QThread(this);
    rs = new ReadSensor();
    rs->moveToThread(rsTh);

    connect(rsTh, &QThread::started, rs, &ReadSensor::start);
    connect(rs, &ReadSensor::newDataSig, this, &MainWindow::updatePlot);
    connect(rs, &ReadSensor::logSig, this, &MainWindow::logMsg);
    connect(this, &MainWindow::finished, rs, &ReadSensor::stop, Qt::DirectConnection);
    connect(rsTh, &QThread::finished, rsTh, &QThread::deleteLater);
    connect(rsTh, &QThread::finished, rs, &ReadSensor::deleteLater);

    rsTh->start();

    setPlot();
}

void MainWindow::logMsg(QString msg){
    ui->logger->appendPlainText(msg);
}

void MainWindow::closeEvent(QCloseEvent *event){
    event->ignore();
    emit finished();

    rsTh->quit();
    rsTh->wait();

//    int userResp = QMessageBox::question(this, "Close Confirmation?", "Are you sure you want to exit?", QMessageBox::Yes|QMessageBox::No);

//    if (QMessageBox::Yes == userResp){
//        event->accept();
//    }
    event->accept();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setPlot()
{
    ui->sensorPlot->plotLayout()->clear();
    ui->sensorPlot->setInteraction(QCP::iRangeDrag, true);
    ui->sensorPlot->setInteraction(QCP::iRangeZoom, true);

    // Position sublayouts
    QCPLayoutGrid *accelLay = new QCPLayoutGrid();
    QCPLayoutGrid *gyroLay  = new QCPLayoutGrid();

    ui->sensorPlot->plotLayout()->addElement(0, 0, accelLay);
    ui->sensorPlot->plotLayout()->addElement(1, 0, gyroLay);

    // Set Time Plots
    for (int i = 0; i < 3; i++){

        QCPAxis *bottomAxis;
        QCPAxis *leftAxis;

        // Accelerometer
        accelPlot[i].axis = new QCPAxisRect(ui->sensorPlot);
        accelPlot[i].axis->setupFullAxesBox(true);
        accelLay->addElement(0, i, accelPlot[i].axis);

        accelPlot[i].timeRange = new QCPRange(0,5);
        accelPlot[i].YRange    = new QCPRange(-2, 2);
        accelPlot[i].axis->axis(QCPAxis::atBottom)->setRange(*accelPlot[i].timeRange);
        accelPlot[i].axis->axis(QCPAxis::atLeft)->setRange(*accelPlot[i].YRange);
        accelPlot[i].axis->axis(QCPAxis::atRight)->setRange(*accelPlot[i].YRange);

        bottomAxis = accelPlot[i].axis->axis(QCPAxis::atBottom);
        leftAxis   = accelPlot[i].axis->axis(QCPAxis::atLeft);
        accelPlot[i].graph = ui->sensorPlot->addGraph(bottomAxis, leftAxis);


        // Gyroscope
        gyroPlot[i].axis = new QCPAxisRect(ui->sensorPlot);
        gyroPlot[i].axis->setupFullAxesBox(true);
        gyroLay->addElement(0, i, gyroPlot[i].axis);

        gyroPlot[i].timeRange = new QCPRange(0,5);
        gyroPlot[i].YRange    = new QCPRange(-300, 300);
        gyroPlot[i].axis->axis(QCPAxis::atBottom)->setRange(*gyroPlot[i].timeRange);
        gyroPlot[i].axis->axis(QCPAxis::atLeft)->setRange(*gyroPlot[i].YRange);
        gyroPlot[i].axis->axis(QCPAxis::atRight)->setRange(*gyroPlot[i].YRange);

        bottomAxis = gyroPlot[i].axis->axis(QCPAxis::atBottom);
        leftAxis   = gyroPlot[i].axis->axis(QCPAxis::atLeft);
        gyroPlot[i].graph = ui->sensorPlot->addGraph(bottomAxis, leftAxis);
    }

    accelPlot[0].axis->axis(QCPAxis::atTop)->setLabel("Accel (X)");
    accelPlot[1].axis->axis(QCPAxis::atTop)->setLabel("Accel (Y)");
    accelPlot[2].axis->axis(QCPAxis::atTop)->setLabel("Accel (Z)");

    gyroPlot[0].axis->axis(QCPAxis::atTop)->setLabel("Gyro (X)");
    gyroPlot[1].axis->axis(QCPAxis::atTop)->setLabel("Gyro (Y)");
    gyroPlot[2].axis->axis(QCPAxis::atTop)->setLabel("Gyro (Z)");
}

void MainWindow::updatePlot(SensorData data)
{
    accelPlot[0].graph->addData(data.time, data.accelX);
    accelPlot[1].graph->addData(data.time, data.accelY);
    accelPlot[2].graph->addData(data.time, data.accelZ);

    gyroPlot[0].graph->addData(data.time, data.gyroX);
    gyroPlot[1].graph->addData(data.time, data.gyroY);
    gyroPlot[2].graph->addData(data.time, data.gyroZ);

    // remove data of lines that's outside visible range:
    double lowerBound = data.time - 4;

    for (int i = 0; i < 3; i++) {
        if (lowerBound > 0) {
            accelPlot[i].axis->axis(QCPAxis::atBottom)->setRange(lowerBound, data.time + 1);
            gyroPlot[i].axis->axis(QCPAxis::atBottom)->setRange(lowerBound, data.time + 1);
        }
    }

    ui->sensorPlot->replot();

}

void MainWindow::on_calibBtn_clicked()
{
    Calib calib(rs, this);
    connect(&calib, &Calib::logSig, this, &MainWindow::logMsg);
    calib.exec();
}
