// C++
#include <iostream>

#include "VDeviceController.h"
#include "FEEControlWidget.h"

//! \class DeviceDAQConnector
DeviceDAQConnector *DeviceDAQConnector::Instance()
{
    static DeviceDAQConnector *instance = new DeviceDAQConnector();
    return instance;
}

DeviceDAQConnector::DeviceDAQConnector()
{
}

DeviceDAQConnector::~DeviceDAQConnector()
{
}

void DeviceDAQConnector::ConnectSlots()
{
    connect(gFEEControlWin, &FEEControlWin::startDAQSignal, this, &DeviceDAQConnector::handle_DAQStart);
    connect(gFEEControlWin, &FEEControlWin::stopDAQSignal, this, &DeviceDAQConnector::handle_DAQDone);

    connect(gFEEControlWin, &FEEControlWin::forceStopDAQSignal, this, &DeviceDAQConnector::forceStopDAQSignal);
}

void DeviceDAQConnector::DisconnectSlots()
{
    disconnect(gFEEControlWin, &FEEControlWin::startDAQSignal, this, &DeviceDAQConnector::handle_DAQStart);
    disconnect(gFEEControlWin, &FEEControlWin::stopDAQSignal, this, &DeviceDAQConnector::handle_DAQDone);

    disconnect(gFEEControlWin, &FEEControlWin::forceStopDAQSignal, this, &DeviceDAQConnector::forceStopDAQSignal);
}

bool DeviceDAQConnector::TryTestPrepare()
{
    if (gFEEControlWin->IsDAQRunning())
    {
        std::cout << "Warning: DAQ is running, cannot prepare for DAQ now, wait for DAQ Done." << std::endl;
        return false;
    }
    ConnectSlots();
    fLastLoopFlag = 0;
    fOccupied = 1;
    return true;
}

void DeviceDAQConnector::TestStop()
{
    DisconnectSlots();
    fBreakFlag = 0;
    fLastLoopFlag = 0;
    fOccupied = 0;
    fDAQhandle = 0;
}

void DeviceDAQConnector::handle_DAQDone()
{
    if (fBreakFlag)
    {
        std::cout << "Warning: Force to break device loop." << std::endl;
        TestStop();
        return;
    }
    if (fLastLoopFlag)
    {
        emit LastDAQDone(fDAQhandle++);
        TestStop();
        return;
    }
    emit DAQDone(fDAQhandle++);
}

void DeviceDAQConnector::handle_DAQStart()
{
    std::cout << "DAQ Start: handle: " << fDAQhandle << std::endl;
}

void DeviceDAQConnector::handle_LastDAQRequest(int deviceHandle, DAQRequestInfo *daq)
{
    fLastLoopFlag = 1;
    handle_DAQRequest(deviceHandle, daq);
}

void DeviceDAQConnector::handle_DAQRequest(int deviceHandle, DAQRequestInfo *daq)
{
    if (fDAQhandle != deviceHandle)
        std::cout << "Warning inside DAQ connector: Handle not match: DAQ handle: " << fDAQhandle << '\t' << "Device request Handle: " << deviceHandle << std::endl;
    std::cout << "Message: Processing DAQ handle: " << fDAQhandle << std::endl;
    // gFEEControlWin->TryStartDAQ(daq->sPath, daq->sFileName, daq->nDAQCount, daq->DAQTime, daq->msBufferSleep, daq->leastBufferEvent);

    std::cout << "daq file name: " << daq->sFileName << std::endl
              << std::endl;
    QTimer timer;
    timer.singleShot(1000, gFEEControlWin, SIGNAL(stopDAQSignal()));
    // _sleep(1000);
    // emit gFEEControlWin->stopDAQSignal();
}

//! \class VDeviceController
void VDeviceController::handle_DAQDone(int daqHandle)
{
    if (fDeviceHandle - 1 != daqHandle)
        std::cout << "Warning inside device controller: Handle not match: DAQ handle: " << daqHandle << '\t' << "Device request DAQ Handle: " << fDeviceHandle - 1 << std::endl;

    if (fStopFlag)
    {
        TestStop();
        return;
    }

    bool flag = ProcessDeviceHandle(fDeviceHandle);

    if (fStopFlag || !flag)
    {
        TestStop();
        return;
    }

    fLastLoopFlag = JudgeLastLoop(fDeviceHandle);
    if (fLastLoopFlag)
    {
        emit RequestForLastDAQ(fDeviceHandle++, &fDAQInfo);
    }
    else
    {
        emit RequestForDAQ(fDeviceHandle++, &fDAQInfo);
    }
}

void VDeviceController::handle_LastDAQDone(int daqHandle)
{
    TestStop();
}

void VDeviceController::handle_ForceStopDAQ()
{
    TestStop();
}

bool VDeviceController::StartTest()
{
    if (fOccupied)
    {
        std::cout << "Warning: Device is occupied, abort" << std::endl;
        return false;
    }
    if (fStopFlag)
    {
        std::cout << "Warning: Stop flag is set to 1 before device start, abort." << std::endl;
        TestStop();
        return false;
    }
    if (!gDAQConnector->TryTestPrepare())
    {
        std::cout << "Warning: DAQ is running, device abort" << std::endl;
        return false;
    }
    bool flag = ProcessDeviceHandle(fDeviceHandle);
    if (fStopFlag || !flag)
    {
        std::cout << "Message: DAQ stop after the first device request." << std::endl;
        std::cout << "fStopFlag: " << fStopFlag << '\t' << "flag: " << flag << std::endl;
        return true;
    }
    fOccupied = 1;
    ConnectSlots();
    emit RequestForDAQ(fDeviceHandle++, &fDAQInfo);
    return true;
}

void VDeviceController::TestStop()
{
    if (fOccupied)
        DisconnectSlots();
    fDeviceHandle = 0;
    fOccupied = 0;
    fStopFlag = 0;
    fLastLoopFlag = 0;
}

void VDeviceController::ConnectSlots()
{
    connect(this, &VDeviceController::RequestForDAQ, gDAQConnector, &DeviceDAQConnector::handle_DAQRequest);
    connect(this, &VDeviceController::RequestForLastDAQ, gDAQConnector, &DeviceDAQConnector::handle_LastDAQRequest);

    connect(gDAQConnector, &DeviceDAQConnector::DAQDone, this, &VDeviceController::handle_DAQDone);
    connect(gDAQConnector, &DeviceDAQConnector::LastDAQDone, this, &VDeviceController::handle_LastDAQDone);

    // Connect force stop daq signal with last daq done slots, tell device to stop
    connect(gDAQConnector, &DeviceDAQConnector::forceStopDAQSignal, this, &VDeviceController::handle_ForceStopDAQ);
}

void VDeviceController::DisconnectSlots()
{
    disconnect(this, &VDeviceController::RequestForDAQ, gDAQConnector, &DeviceDAQConnector::handle_DAQRequest);
    disconnect(this, &VDeviceController::RequestForLastDAQ, gDAQConnector, &DeviceDAQConnector::handle_LastDAQRequest);

    disconnect(gDAQConnector, &DeviceDAQConnector::DAQDone, this, &VDeviceController::handle_DAQDone);
    disconnect(gDAQConnector, &DeviceDAQConnector::LastDAQDone, this, &VDeviceController::handle_LastDAQDone);
    disconnect(gDAQConnector, &DeviceDAQConnector::forceStopDAQSignal, this, &VDeviceController::handle_ForceStopDAQ);
}

bool TestDevice::ProcessDeviceHandle(int deviceHandle)
{
    std::cout << "Message: Processing Device Handle: " << '\t' << deviceHandle << std::endl;
    if (deviceHandle > 10)
    {
        fStopFlag = 1;
        return false;
    }
    return true;
}

void VDeviceController::ForceStopDevice()
{
    fStopFlag = 1;
    gDAQConnector->SetBreak();
    TestStop();
}
