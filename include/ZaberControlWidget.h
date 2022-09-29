#ifndef ZABERCONTROLWIDGET_H
#define ZABERCONTROLWIDGET_H

#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <queue>

#define gZaberWindow (ZaberControlWidget::Instance())

class QListWidgetItem;

namespace Ui
{
    class ZaberControlWidget;
}

class ZaberControlWidget;
class PlotWindow;
class FTAnalyzerWin;

// Zaber Move Processing class
class DeviceMove : public QObject
{
    Q_OBJECT
signals:
    void moveReady();

public slots:
    void startMove(ZaberControlWidget *w);
};

class ZaberControlWidget : public QMainWindow
{
    Q_OBJECT

    friend class DeviceMove;
    friend class DAQRuning;
    friend class FiberTestDrawing;
    friend class ZaberFunctionTestClass;

signals:
    void startRunning(ZaberControlWidget *w); // Zaber start runing signal
    void moveRequestDone(int handle);         // emit move request done signal, requst handle should be the same as start
    void moveRequestStart(int handle);        // emitted when move request start, emit request handle

public slots:
    void on_RunningStoped();             // Zaber Stop Slot
    void handle_MoveRequest(double pos); // handle move request from other classes

public:
    ~ZaberControlWidget();
    static ZaberControlWidget *ZaberControlWidget::Instance();

    double getPosition() { return monitorValue; }
    bool isValid() { return !(fDevIndex < 0); }

private:
    Ui::ZaberControlWidget *ui;
    explicit ZaberControlWidget(QWidget *parent = nullptr);

    // Process Running
    void startOneRunning(double pos);

    // Zaber Motion
    void ScanPorts();
    void ProcessDeviceList();
    void ProcessMonitorValue();
    void ProcessMoveSetValue();

    double monitorValue;            // position monitor value
    double moveSetValue;            // only useful for saving move value, in mm unit
    const int maxSliderValue = 100; // maximum slider bar value
    const double maxSetValue = 200; // maximum moveSetValue , 200mm

    DeviceMove *fMoveWorker; // Process time cosuming processes
    QThread fWorkThread1;    //
    QTimer fMonitorTimer;    // Timer for monitor

    bool fDevRunningFlag = 0;    // Whether device is occupied
    bool fMonitorRuningFlag = 0; // Control when the Zaber monitor stops
    int fDevIndex = -1;          // active device

    int fProcessingHandle = 0;      // Request handle that is going to process
    double fProcessingPos = 0;      // Move Destination position
    int fNextHandle = 0;            // Next handle
    std::queue<double> fRequestPos; // Requested position
    std::queue<double> fHandles;    // Requested handles

private slots:
    void updateMonitor(); // Update monitor

private slots:
    void on_btnScanPort_clicked();
    void on_btnScanDevice_clicked();
    void on_btnAutoScanDevice_clicked();
    void on_boxUnits_currentIndexChanged(int index);
    void on_boxSetMovVal_valueChanged(double arg1);
    void on_sliderPosition_actionTriggered(int action);
    void on_listDevice_itemActivated(QListWidgetItem *item);
    void on_btnMove_clicked();
    void on_sliderMonitor_actionTriggered(int action);
    void on_btnMonitor_clicked();
};

#include <iostream>
class ZaberTest : public QObject
{
    Q_OBJECT
public:
    ZaberTest(double start, double end, double step);
    bool SetZaberPar(double start, double end, double step);
    bool IsOccupied() { return fOccupied; }

private:
    int gHandle = 0;

    double fposStart = 0;
    double fposEnd = 0;
    double fstep = 0;
    bool fOccupied = 0;

public slots:
    void startMove();
    void handle_MoveDone(int handle);
    void handle_MoveStart(int handle);

    void handle_DAQStart();
    void handle_DAQDone();
};

#endif // ZABERCONTROLWIDGET_H
