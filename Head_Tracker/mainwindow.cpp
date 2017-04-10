#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    rsTh = new QThread(this);
    rs = new ReadSensor();
    rs->moveToThread(rsTh);

    connect(rsTh, &QThread::started, rs, &ReadSensor::start);
    connect(rs, &ReadSensor::logSig, this, &MainWindow::logMsg);
    connect(rsTh, &QThread::finished, rs, &ReadSensor::deleteLater);
    connect(rsTh, &QThread::finished, rsTh, &QThread::deleteLater);

    rsTh->start();
}

void MainWindow::logMsg(QString msg){
    ui->logger->appendPlainText(msg);
}


void MainWindow::closeEvent(QCloseEvent *event){
    event->ignore();

    rsTh->quit();
    rsTh->wait();

    int userResp = QMessageBox::question(this, "Close Confirmation?", "Are you sure you want to exit?", QMessageBox::Yes|QMessageBox::No);

    if (QMessageBox::Yes == userResp){
        event->accept();
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}
