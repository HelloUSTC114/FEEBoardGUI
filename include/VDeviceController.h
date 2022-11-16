#ifndef VDEVICECONTROLLER_H
#define VDEVICECONTROLLER_H

#include <QObject>
#include <QTime>

#include "General.h"

// DAQ Controller and device contoller

class DeviceDAQConnector : public QObject
{
    Q_OBJECT
public:
    static DeviceDAQConnector *Instance();
    ~DeviceDAQConnector();
    bool IsOccupied() { return fOccupied; };
    void SetBreak(); // Force DAQ Stop
    void ClearBreak() { fManualForceBreak = 0; };

    bool TryTestPrepare(); // connect DAQ signals inside FEEControlWidget and slots in this class
    void TestStop();       // Disconnect singlas and slots

private:
    DeviceDAQConnector();
    void ConnectSlots();
    void DisconnectSlots();

    volatile bool fOccupied = 0;
    volatile bool fManualForceBreak = 0;
    volatile bool fLastLoopFlag = 0;

    int fDAQhandle = 0;

signals:
    void DAQDone(int daqHandle); // Whole process is controlled by device controller
    void LastDAQDone(int daqHandle);

    void forceStopDAQSignal(); // emit DAQ Force stop signal from  FEEControlWin, tell other class that DAQ is interrupted

    void DirectRequestForWidgetDAQ(UserDefine::DAQRequestInfo *daq); // DAQ Request signal

public slots:
    void handle_DAQRequest(int deviceHandle, UserDefine::DAQRequestInfo *daq);
    void handle_LastDAQRequest(int deviceHandle, UserDefine::DAQRequestInfo *daq);

private slots:
    void handle_DAQStart();
    void handle_DAQDone();

    void handle_ForceStopDAQ(); // handle stop signal from FEEControlWin
};

#define gDAQConnector (DeviceDAQConnector::Instance())

/// @brief pure virtual device controller
class VDeviceController : public QObject
{
    Q_OBJECT
public:
    virtual bool StartTest();
    virtual void ForceStopDevice();

signals:
    void RequestForDAQ(int daqHandle, UserDefine::DAQRequestInfo *daq);
    void RequestForLastDAQ(int daqHandle, UserDefine::DAQRequestInfo *daq);
    void RequestForForceStop();

    void deviceStarted(QPrivateSignal);
    void deviceStoped(QPrivateSignal);

public slots:
    virtual void handle_DAQDone(int daqHandle);
    virtual void handle_LastDAQDone(int daqHandle);
    virtual void handle_ForceStopDAQ(); // handle stop signal from FEEControlWin

protected:
    volatile int fDeviceHandle = 0;
    volatile bool fOccupied = 0;
    volatile bool fStopFlag = 0;
    volatile bool fLastLoopFlag = 0;

    UserDefine::DAQRequestInfo fDAQInfo;

    virtual void TestStop();
    virtual bool ProcessDeviceHandle(int deviceHandle) = 0;
    virtual bool JudgeLastLoop(int deviceHandle) = 0;

    virtual void ConnectSlots();
    virtual void DisconnectSlots();
};

class TestDevice : public VDeviceController
{
    Q_OBJECT

public:
    virtual bool ProcessDeviceHandle(int deviceHandle) override;
    virtual bool JudgeLastLoop(int deviceHandle) override;
};
#endif // VDEVICECONTROLLER_H
