#include "calibration.h"
#include "ui_calibration.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <opencv2/core/types.hpp>

using namespace cv;

Calib::Calib(ReadSensor *rs, QWidget *parent) : QDialog(parent), ui(new Ui::CalibDlg)
{
    ui->setupUi(this);

    QString calibPath = QCoreApplication::applicationDirPath() + "/calib.txt";
    ui->savePathEdit->setText(calibPath);   // Pre-define calib saving path to current program folder

    this->rs = rs;
    numSamples = 100;                   // # measurement samples per calibration point
    const int numCols = 6 * numSamples; // 6-pt tumble calibration (3 axis * 2 directions)
    colIdx = 0;

    accelMeas       = Mat_<double>::zeros(3,numCols); // Actual measurements
    gyroMeas        = Mat_<double>::zeros(3,numCols);
    accelReal       = Mat_<double>::zeros(4,numCols); // Desired/Real measurement values
    gyroReal        = Mat_<double>::zeros(4,numCols);
    currAccelState  = Mat_<double>::zeros(3,1);       // Real mesurement values for current calib point (last row of 1's is added later on)
    currGyroState   = Mat_<double>::zeros(3,1);

    ui->saveBtn->setEnabled(false); // Enable save btn when all points are recorded
}

void Calib::on_xUpBtn_clicked()
{
    // X-axis opposite direction than gravity
    currAccelState(0) = -1.0;
    currAccelState(1) = 0.0;
    currAccelState(2) = 0.0;
    ui->xUpBtn->setEnabled(false);

    startRecording();
}

void Calib::on_xDownBtn_clicked()
{
    // X-axis same direction than gravity
    currAccelState(0) = 1.0;
    currAccelState(1) = 0.0;
    currAccelState(2) = 0.0;
    ui->xDownBtn->setEnabled(false);

    startRecording();
}

void Calib::on_yUpBtn_clicked()
{
    // Y-axis opposite direction than gravity
    currAccelState(0) = 0.0;
    currAccelState(1) = -1.0;
    currAccelState(2) = 0.0;
    ui->yUpBtn->setEnabled(false);

    startRecording();
}

void Calib::on_yDownBtn_clicked()
{
    // Y-axis same direction than gravity
    currAccelState(0) = 0.0;
    currAccelState(1) = 1.0;
    currAccelState(2) = 0.0;
    ui->yDownBtn->setEnabled(false);

    startRecording();
}

void Calib::on_zUpBtn_clicked()
{
    // Z-axis opposite direction than gravity
    currAccelState(0) = 0.0;
    currAccelState(1) = 0.0;
    currAccelState(2) = -1.0;
    ui->zUpBtn->setEnabled(false);

    startRecording();
}

void Calib::on_zDownBtn_clicked()
{
    // Z-axis same direction than gravity
    currAccelState(0) = 0.0;
    currAccelState(1) = 0.0;
    currAccelState(2) = 1.0;
    ui->zDownBtn->setEnabled(false);

    startRecording();
}

void Calib::startRecording()
{
    cnter = numSamples;
    ui->recordLed->setStyleSheet("background-color: red");  // Set LED to red to indicate recording in-progress
    connect(rs, &ReadSensor::newDataSig, this, &Calib::processData);
}

void Calib::processData(SensorData data)
{
    // Append new actual measurement
    accelMeas(0, colIdx) = data.accelX;
    accelMeas(1, colIdx) = data.accelY;
    accelMeas(2, colIdx) = data.accelZ;

    // Append desired/real measurement
    accelReal(0, colIdx) = currAccelState(0);
    accelReal(1, colIdx) = currAccelState(1);
    accelReal(2, colIdx) = currAccelState(2);
    accelReal(3, colIdx) = 1.0;     // Needed for the algo to work (for offset estimation)

    colIdx++;
    cnter--;

    if (cnter == 0){
        disconnect(rs, &ReadSensor::newDataSig, this, &Calib::processData);
        ui->recordLed->setStyleSheet("background-color: rgb(0, 255, 127)"); // Set LED back to green to indicate readiness for another point

        // Verify if all points have been recorded
        if (!ui->xUpBtn->isEnabled() && !ui->xDownBtn->isEnabled() &&
            !ui->yUpBtn->isEnabled() && !ui->yDownBtn->isEnabled() &&
            !ui->zUpBtn->isEnabled() && !ui->zDownBtn->isEnabled() )
        {
            QMessageBox::information(this, "Completed", "6-pts tumble calib is completed!");
            ui->saveBtn->setEnabled(true);  // Enable saving of calibration parameters
        }
    }
}

void Calib::on_saveBtn_clicked()
{
    // Estimate the Gain and Offset for each axis
    Mat_<double> A = accelReal * accelReal.t();
    Mat_<double> accelGainOffset = accelMeas * accelReal.t() * A.inv();

    // Separate Gain and Offset
    Rect gainROI(0,0,3,3);
    Rect offsetROI(3,0,1,3);

    Mat gain = accelGainOffset(gainROI);
    Mat offset = accelGainOffset(offsetROI);

    emit logSig("gain");
    printMat(gain);
    emit logSig("offset");
    printMat(offset);

    /**********************************
     * TODO: Estimate Gain+Offset for Gyro
     * *********************************/
    Mat_<double> gyroGainOffset = Mat_<double>::zeros(3,4);
    gyroGainOffset(0,0) = 1.0;
    gyroGainOffset(1,1) = 1.0;
    gyroGainOffset(2,2) = 1.0;

    /*************************************/

    // Save gain+offset to file
    QFile calibFile(ui->savePathEdit->text());
    QTextStream out(&calibFile);

    if (!calibFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit QMessageBox::critical(this, "ERROR", "Could not write to calib file. Verify its path and retry!");

    }
    else {
        out << accelGainOffset(0,0) << "," << accelGainOffset(0,1)  << "," << accelGainOffset(0,2)  << "," << accelGainOffset(0,3)  << ","
            << gyroGainOffset(0,0)  << "," << gyroGainOffset(0,1)   << "," << gyroGainOffset(0,2)   << "," << gyroGainOffset(0,3)   << "\n"
            << accelGainOffset(1,0) << "," << accelGainOffset(1,1)  << "," << accelGainOffset(1,2)  << "," << accelGainOffset(1,3)  << ","
            << gyroGainOffset(1,0)  << "," << gyroGainOffset(1,1)   << "," << gyroGainOffset(1,2)   << "," << gyroGainOffset(1,3)   << "\n"
            << accelGainOffset(2,0) << "," << accelGainOffset(2,1)  << "," << accelGainOffset(2,2)  << "," << accelGainOffset(2,3)  << ","
            << gyroGainOffset(2,0)  << "," << gyroGainOffset(2,1)   << "," << gyroGainOffset(2,2)   << "," << gyroGainOffset(2,3)   << "\n";

        this->accept();
    }

    calibFile.close();
}

void Calib::on_cancelBtn_clicked()
{
    this->reject();
}

Calib::~Calib(){
    delete ui;
    accelMeas.release();
    accelReal.release();
    currAccelState.release();

    gyroMeas.release();
    gyroReal.release();
    currGyroState.release();
}

void Calib::printMat(cv::Mat_<double> mat)
{
    QString out = "";

    for (int i = 0; i < mat.rows; i++) {

        for (int j = 0; j < mat.cols; j++){
            out += QString::number(mat(i,j)) + " ";
        }

        out += "\n";
    }
    out += "\n";
    emit logSig(out);
}






