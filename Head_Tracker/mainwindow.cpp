#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calibration.h"

#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(this, &MainWindow::logSig, this, &MainWindow::logMsg);

    rsTh = new QThread(this);
    rs = new readImuPi();
    rs->moveToThread(rsTh);

    connect(rsTh, &QThread::started, rs, &readImuPi::start);
    connect(rs, &readImuPi::newDataSig, this, &MainWindow::processData);
    connect(this, &MainWindow::procDataSig, this, &MainWindow::updatePlot);
    connect(rs, &readImuPi::logSig, this, &MainWindow::logMsg);
    connect(this, &MainWindow::finished, rs, &readImuPi::stop, Qt::DirectConnection);
    connect(rsTh, &QThread::finished, rsTh, &QThread::deleteLater);
    connect(rsTh, &QThread::finished, rs, &ReadTds::deleteLater);

    rsTh->start();
    setPlot();

    accelGain   = cv::Matx33d::eye();
    gyroGain    = cv::Matx33d::eye();
    magGain    = cv::Matx33d::eye();
    accelOffset = cv::Matx31d::zeros();
    gyroOffset  = cv::Matx31d::zeros();
    magOffset  = cv::Matx31d::zeros();
}

void MainWindow::logMsg(QString msg){
    ui->logger->appendPlainText(msg);
}

void MainWindow::processData(SensorData data)
{
    // Calibrate scaled ACCEL values
    cv::Matx31d accelMeas;
    accelMeas(0) = data.accelX;
    accelMeas(1) = data.accelY;
    accelMeas(2) = data.accelZ;
    cv::Matx31d calibAccel = accelGain * accelMeas + accelOffset;

    // Calibrate scaled GYRO values
    cv::Matx31d gyroMeas;
    gyroMeas(0) = data.gyroX;
    gyroMeas(1) = data.gyroY;
    gyroMeas(2) = data.gyroZ;
    cv::Matx31d calibGyro = gyroGain * gyroMeas + gyroOffset;

    // Calibrate scaled MAG values
    cv::Matx31d magMeas;
    magMeas(0) = data.magX;
    magMeas(1) = data.magY;
    magMeas(2) = data.magZ;
    cv::Matx31d calibMag = magGain * magMeas + magOffset;

    // Send back a calibrated signal
    data.accelX = calibAccel(0);
    data.accelY = calibAccel(1);
    data.accelZ = calibAccel(2);

    data.gyroX = calibGyro(0);
    data.gyroY = calibGyro(1);
    data.gyroZ = calibGyro(2);

    data.magX = calibMag(0);
    data.magY = calibMag(1);
    data.magZ = calibMag(2);

    emit procDataSig(data);
}

void MainWindow::on_deviceIdCombo_currentIndexChanged(int index)
{
    if (index == 0){
        emit logSig("Displaying RAW data.......");
        accelGain   = cv::Matx33d::eye();
        gyroGain    = cv::Matx33d::eye();
        magGain     = cv::Matx33d::eye();
        accelOffset = cv::Matx31d::zeros();
        gyroOffset  = cv::Matx31d::zeros();
        magOffset   = cv::Matx31d::zeros();
        return;
    }

    emit logSig("Loading Calibration parameters.......");
    QString calibPath = QString("%1/calib_%2.txt").arg(QCoreApplication::applicationDirPath()).arg(index);
    QFile calibFile(calibPath);
    QTextStream stream(&calibFile);

    if (!calibFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString msg = "Could not read the calib file located in:\n" + calibPath;
        emit QMessageBox::critical(this, "Open File Error", msg);
        return;
    }

    // Read calib to temporary matrices in case of read errors occurring
    // so instance variables would only be updated if loading is a success
    cv::Matx33d _accelGain   = cv::Matx33d::zeros();
    cv::Matx33d _gyroGain    = cv::Matx33d::zeros();
    cv::Matx33d _magGain     = cv::Matx33d::zeros();
    cv::Matx31d _accelOffset = cv::Matx31d::zeros();
    cv::Matx31d _gyroOffset  = cv::Matx31d::zeros();
    cv::Matx31d _magOffset   = cv::Matx31d::zeros();

    int rowIdx = 0;
    bool abort = false;

    while(!stream.atEnd()) {

        if (rowIdx == 3) {
            QString msg = "Abort: More rows than expected in the calib file:\n" + calibPath;
            emit QMessageBox::critical(this, "Corrupted File", msg);
            abort = true;
            break;
        }

        QStringList row = stream.readLine().split(",");

        if (row.size() != 0 && row.size() != 12) {
            QString msg = "Abort: The following calib file is corrupted:\n" + calibPath;
            emit QMessageBox::critical(this, "Corrupted File", msg);
            abort = true;
            break;
        }
        else if (row.size() == 12){
            _accelGain(rowIdx,0) = ((QString)row[0]).toDouble();
            _accelGain(rowIdx,1) = ((QString)row[1]).toDouble();
            _accelGain(rowIdx,2) = ((QString)row[2]).toDouble();
            _accelOffset(rowIdx) = ((QString)row[3]).toDouble();

            _gyroGain(rowIdx,0) = ((QString)row[4]).toDouble();
            _gyroGain(rowIdx,1) = ((QString)row[5]).toDouble();
            _gyroGain(rowIdx,2) = ((QString)row[6]).toDouble();
            _gyroOffset(rowIdx) = ((QString)row[7]).toDouble();

            _magGain(rowIdx,0) = ((QString)row[8]).toDouble();
            _magGain(rowIdx,1) = ((QString)row[9]).toDouble();
            _magGain(rowIdx,2) = ((QString)row[10]).toDouble();
            _magOffset(rowIdx) = ((QString)row[11]).toDouble();
            rowIdx++;
        }
    }

    calibFile.close();

    if (!abort) {
        accelGain   = _accelGain;
        accelOffset = _accelOffset;

        gyroGain   = _gyroGain;
        gyroOffset = _gyroOffset;

        magGain   = _magGain;
        magOffset = _magOffset;

        QString accelGainLog = "\nAccel Gain:\n" + printMat(cv::Mat(accelGain));
        emit logSig(accelGainLog);

        QString accelOffsetLog = "Accel Offset:" + printMat(cv::Mat(accelOffset.t())); // Make it read in 1 row
        emit logSig(accelOffsetLog);

        QString gyroGainLog = "\nGyro Gain:\n" + printMat(cv::Mat(gyroGain));
        emit logSig(gyroGainLog);

        QString gyroOffsetLog = "Gyro Offset:" + printMat(cv::Mat(gyroOffset.t())); // Make it read in 1 row
        emit logSig(gyroOffsetLog);

        QString magGainLog = "\nMag Gain:\n" + printMat(cv::Mat(magGain));
        emit logSig(magGainLog);

        QString magOffsetLog = "Mag Offset:" + printMat(cv::Mat(magOffset.t())); // Make it read in 1 row
        emit logSig(magOffsetLog);
    }
}

void MainWindow::on_calibBtn_clicked()
{
//    Calib calib(rs, this);
//    connect(&calib, &Calib::logSig, this, &MainWindow::logMsg);
//    calib.exec();
}

void MainWindow::setPlot()
{
    ui->sensorPlot->plotLayout()->clear();
    ui->sensorPlot->setInteraction(QCP::iRangeDrag, true);
    ui->sensorPlot->setInteraction(QCP::iRangeZoom, true);

    // Position sublayouts
    QCPLayoutGrid *accelLay = new QCPLayoutGrid();
    QCPLayoutGrid *gyroLay  = new QCPLayoutGrid();
    QCPLayoutGrid *magLay   = new QCPLayoutGrid();

    ui->sensorPlot->plotLayout()->addElement(0, 0, accelLay);
    ui->sensorPlot->plotLayout()->addElement(1, 0, gyroLay);
    ui->sensorPlot->plotLayout()->addElement(2, 0, magLay);

    // Set Time Plots
    for (int i = 0; i < 3; i++){

        QCPAxis *bottomAxis;
        QCPAxis *leftAxis;

        // Accelerometer
        accelPlot[i].axis = new QCPAxisRect(ui->sensorPlot);
        accelPlot[i].axis->setupFullAxesBox(true);
        accelLay->addElement(0, i, accelPlot[i].axis);

        accelPlot[i].timeRange = new QCPRange(0,5);
        accelPlot[i].YRange    = new QCPRange(-3,3); // Max range = 2G
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
        gyroPlot[i].YRange    = new QCPRange(-250,250); // Max range = 245 deg/s
        gyroPlot[i].axis->axis(QCPAxis::atBottom)->setRange(*gyroPlot[i].timeRange);
        gyroPlot[i].axis->axis(QCPAxis::atLeft)->setRange(*gyroPlot[i].YRange);
        gyroPlot[i].axis->axis(QCPAxis::atRight)->setRange(*gyroPlot[i].YRange);

        bottomAxis = gyroPlot[i].axis->axis(QCPAxis::atBottom);
        leftAxis   = gyroPlot[i].axis->axis(QCPAxis::atLeft);
        gyroPlot[i].graph = ui->sensorPlot->addGraph(bottomAxis, leftAxis);


        // Magnetometer
        magPlot[i].axis = new QCPAxisRect(ui->sensorPlot);
        magPlot[i].axis->setupFullAxesBox(true);
        magLay->addElement(0, i, magPlot[i].axis);

        magPlot[i].timeRange = new QCPRange(0,5);
        magPlot[i].YRange    = new QCPRange(-5,5);  // Max range = 4 gauss
        magPlot[i].axis->axis(QCPAxis::atBottom)->setRange(*magPlot[i].timeRange);
        magPlot[i].axis->axis(QCPAxis::atLeft)->setRange(*magPlot[i].YRange);
        magPlot[i].axis->axis(QCPAxis::atRight)->setRange(*magPlot[i].YRange);

        bottomAxis = magPlot[i].axis->axis(QCPAxis::atBottom);
        leftAxis   = magPlot[i].axis->axis(QCPAxis::atLeft);
        magPlot[i].graph = ui->sensorPlot->addGraph(bottomAxis, leftAxis);
    }

    accelPlot[0].axis->axis(QCPAxis::atTop)->setLabel("Accel (X)");
    accelPlot[1].axis->axis(QCPAxis::atTop)->setLabel("Accel (Y)");
    accelPlot[2].axis->axis(QCPAxis::atTop)->setLabel("Accel (Z)");

    gyroPlot[0].axis->axis(QCPAxis::atTop)->setLabel("Gyro (X)");
    gyroPlot[1].axis->axis(QCPAxis::atTop)->setLabel("Gyro (Y)");
    gyroPlot[2].axis->axis(QCPAxis::atTop)->setLabel("Gyro (Z)");

    magPlot[0].axis->axis(QCPAxis::atTop)->setLabel("Mag (X)");
    magPlot[1].axis->axis(QCPAxis::atTop)->setLabel("Mag (Y)");
    magPlot[2].axis->axis(QCPAxis::atTop)->setLabel("Mag (Z)");
}

void MainWindow::updatePlot(SensorData data)
{
    accelPlot[0].graph->addData(data.time, data.accelX);
    accelPlot[1].graph->addData(data.time, data.accelY);
    accelPlot[2].graph->addData(data.time, data.accelZ);

    gyroPlot[0].graph->addData(data.time, data.gyroX);
    gyroPlot[1].graph->addData(data.time, data.gyroY);
    gyroPlot[2].graph->addData(data.time, data.gyroZ);

    magPlot[0].graph->addData(data.time, data.magX);
    magPlot[1].graph->addData(data.time, data.magY);
    magPlot[2].graph->addData(data.time, data.magZ);

    // remove data of lines that's outside visible range:
    double lowerBound = data.time - 4;

    for (int i = 0; i < 3; i++) {
        if (lowerBound > 0) {
            accelPlot[i].axis->axis(QCPAxis::atBottom)->setRange(lowerBound, data.time + 1);
            gyroPlot[i].axis->axis(QCPAxis::atBottom)->setRange(lowerBound, data.time + 1);
            magPlot[i].axis->axis(QCPAxis::atBottom)->setRange(lowerBound, data.time + 1);
        }
    }

    ui->sensorPlot->replot();

}

QString MainWindow::printMat(cv::Mat mat)
{
    QString out = "";

    for (int i = 0; i < mat.rows; i++) {

        for (int j = 0; j < mat.cols; j++){
            out += QString::number(mat.at<double>(i,j)) + " ";
        }

        out += "\n";
    }
    out += "\n";
    return out;
}

void MainWindow::closeEvent(QCloseEvent *event){
    event->ignore();
    emit finished();
    rsTh->quit();
    rsTh->wait();
    event->accept();
}

MainWindow::~MainWindow()
{
    delete ui;
}
