#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "readsensor.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void closeEvent(QCloseEvent * event);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QThread *rsTh;
    ReadSensor *rs;

private slots:
    void logMsg(QString msg);

signals:
    void logSig(QString);
    void finished();
};

#endif // MAINWINDOW_H
