#ifndef VDEVICECONTROLLER_H
#define VDEVICECONTROLLER_H

#include <QObject>
#include <QTime>
struct DAQRequestInfo
{
    std::string sPath;
    std::string sFileName;
    int nDAQCount;
    QTime DAQTime;
    int msBufferSleep;
    int leastBufferEvent;
};

// DAQ Controller and device contoller

struct DAQRequestInfo;

class DeviceDAQConnector : public QObject
{
    Q_OBJECT
public:
    static DeviceDAQConnector *Instance();
    ~DeviceDAQConnector();
    bool IsOccupied() { return fOccupied; };
    void SetBreak() { fBreakFlag = 1; };

    bool TryTestPrepare(); // connect DAQ signals inside FEEControlWidget and slots in this class
    void TestStop();       // Disconnect singlas and slots

private:
    DeviceDAQConnector();
    void ConnectSlots();
    void DisconnectSlots();

    bool fOccupied = 0;
    volatile bool fBreakFlag = 0;
    volatile bool fLastLoopFlag = 0;

    int fDAQhandle = 0;

signals:
    void DAQDone(int daqHandle); // Whole process is controlled by device controller
    void LastDAQDone(int daqHandle);

public slots:
    void handle_DAQRequest(int deviceHandle, DAQRequestInfo *daq);
    void handle_LastDAQRequest(int deviceHandle, DAQRequestInfo *daq);

private slots:
    void handle_DAQStart();
    void handle_DAQDone();
};

#define gDAQConnector (DeviceDAQConnector::Instance())

/// @brief pure virtual device controller
class VDeviceController : public QObject
{
    Q_OBJECT
public:
    virtual bool StartTest();
    virtual void ForceStopDevice() { fStopFlag = 1; };

signals:
    void RequestForDAQ(int daqHandle, DAQRequestInfo *daq);
    void RequestForLastDAQ(int daqHandle, DAQRequestInfo *daq);

public slots:
    virtual void handle_DAQDone(int daqHandle);
    virtual void handle_LastDAQDone(int daqHandle);

protected:
    int fDeviceHandle = 0;
    bool fOccupied = 0;
    volatile bool fStopFlag = 0;
    volatile bool fLastLoopFlag = 0;

    DAQRequestInfo fDAQInfo;

    virtual void TestStop();
    virtual bool ProcessDeviceHandle(int deviceHandle) = 0;

    virtual void ConnectSlots();
    virtual void DisconnectSlots();
};

class TestDevice : public VDeviceController
{
    Q_OBJECT

public:
    virtual bool ProcessDeviceHandle(int deviceHandle) override;
};
#endif // VDEVICECONTROLLER_H
