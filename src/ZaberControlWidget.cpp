#include "ZaberControlWidget.h"
#include "ui_ZaberControlWidget.h"

// ROOT
#include "TH1D.h"

// User
#include "ZaberConnectionManager.h"
#include "ROOTDraw.h"
#include "feecontrol.h"
#include "configfileparser.h"
#include "datamanager.h"

// QT
#include <QDir>
#include <QFileDialog>
#include <QDateTime>
#include <QStringList>
#include <QMessageBox>

using namespace std;
using namespace zaber::motion::binary;
using namespace zaber::motion;

ZaberControlWidget::ZaberControlWidget(QWidget *parent) : QMainWindow(parent),
                                                          ui(new Ui::ZaberControlWidget)
{
    ui->setupUi(this);

    // Zaber control tab
    ScanPorts();
    moveSetValue = 0;
    ui->sliderPosition->setValue(moveSetValue);
    ui->sliderPosition->setRange(0, maxSliderValue);
    ui->boxSetMovVal->setValue(moveSetValue);
    ui->boxSetMovVal->setRange(0, maxSetValue);
    ui->boxSetMovVal->setDecimals(4);
    ui->boxUnits->setCurrentIndex(0);

    ui->grpZaberControl->setEnabled(false);

    // zaber monitor thread
    fMoveWorker = new DeviceMove();
    fMoveWorker->moveToThread(&fWorkThread1);
    connect(this, &ZaberControlWidget::startRunning, fMoveWorker, &DeviceMove::startMove);
    connect(&fWorkThread1, &QThread::finished, fMoveWorker, &QObject::deleteLater);
    connect(fMoveWorker, &DeviceMove::moveReady, this, &ZaberControlWidget::on_RunningStoped);

    // connect(this, &ZaberControlWidget::startRunning, fMoveMonitor, &MoveMonitor::startMonitor);
    connect(&fMonitorTimer, &QTimer::timeout, this, &ZaberControlWidget::updateMonitor);

    // Process move request from other classes

    // MultiThread start
    fWorkThread1.start();
}

void ZaberControlWidget::updateMonitor()
{
    auto dev = gconm->getDeviceList()[fDevIndex];
    auto pos = dev.getPosition(zaber::motion::Units::LENGTH_MILLIMETRES);
    monitorValue = pos;
    std::cout << "Test: Monitor: " << pos << std::endl;
    ProcessMonitorValue();
}

ZaberControlWidget *ZaberControlWidget::Instance()
{
    static ZaberControlWidget *instance = new ZaberControlWidget(NULL);
    return instance;
}

ZaberControlWidget::~ZaberControlWidget()
{
    delete ui;

    fWorkThread1.quit();
    fWorkThread1.wait();
}

void ZaberControlWidget::handle_MoveRequest(double pos)
{
    if (fDevIndex < 0)
        return;
    if (fDevRunningFlag)
    {
        fHandles.push(fNextHandle);
        fRequestPos.push(pos);
        emit moveRequestStart(fNextHandle);
    }
    else
    {
        startOneRunning(pos);
        emit moveRequestStart(fProcessingHandle);
    }
    fNextHandle++;
}

void ZaberControlWidget::ScanPorts()
{
    ui->portBox->clear();

    auto portList = getComPort();
    for (int i = 0; i < portList.size(); i++)
    {
        ui->portBox->addItem(tr(portList[i].c_str()));
    }
}

void ZaberControlWidget::ProcessDeviceList()
{
    ui->listDevice->clear();
    auto deviceList = gconm->getDeviceList();
    for (int i = 0; i < deviceList.size(); i++)
    {
        ui->listDevice->addItem(tr(deviceList[i].getName().c_str()));
    }
}

void ZaberControlWidget::on_btnScanPort_clicked()
{
    ScanPorts();
    QMessageBox::information(NULL, tr("Message"), tr("Rescaned ports!"));
}

void ZaberControlWidget::on_btnScanDevice_clicked()
{
    ui->listDevice->clear();

    bool flag = gconm->ScanPort(ui->portBox->currentText().toStdString());
    if (!flag)
    {
        QMessageBox::information(NULL, tr("Warning"), tr("Find no device!"));
        return;
    }

    ProcessDeviceList();
}

void ZaberControlWidget::on_btnAutoScanDevice_clicked()
{
    ui->listDevice->clear();

    string portName = gconm->ScanDevice();
    if (portName == UserDefine::NO_DEVICE_FOUND)
    {
        QMessageBox::information(NULL, tr("Warning!"), tr("Find no device!"));
        return;
    }

    int index = ui->portBox->findText(tr(portName.c_str()));
    if (index >= 0)
    {
        ui->portBox->setCurrentIndex(index);
    }
    else
    {
        ui->portBox->addItem(tr(portName.c_str()));
        ui->portBox->setCurrentIndex(ui->portBox->count());
    }

    ProcessDeviceList();
}

void ZaberControlWidget::on_boxUnits_currentIndexChanged(int index)
{
    if (ui->boxUnits->currentText() == tr("cm"))
    {
        ui->boxSetMovVal->setValue(moveSetValue / 10);
        ui->boxSetMovVal->setRange(0, 20);
    }
    else if (ui->boxUnits->currentText() == tr("mm"))
    {
        ui->boxSetMovVal->setRange(0, 200);
        ui->boxSetMovVal->setValue(moveSetValue);
    }
    else
        cout << "Error of units" << endl;
}

void ZaberControlWidget::on_boxSetMovVal_valueChanged(double arg1)
{
    if (ui->boxUnits->currentText() == tr("cm"))
        moveSetValue = arg1 * 10;
    else if (ui->boxUnits->currentText() == tr("mm"))
        moveSetValue = arg1;
    else
        cout << "Error of units" << endl;

    ui->sliderPosition->setValue(moveSetValue / maxSetValue * maxSliderValue);
}

void ZaberControlWidget::on_sliderPosition_actionTriggered(int action)
{
    moveSetValue = (double)ui->sliderPosition->value() / maxSliderValue * maxSetValue; // Unit: mm

    if (ui->boxUnits->currentText() == tr("cm"))
    {
        ui->boxSetMovVal->setValue(moveSetValue / 10);
        ui->boxSetMovVal->setRange(0, 20);
    }
    else if (ui->boxUnits->currentText() == tr("mm"))
    {
        ui->boxSetMovVal->setValue(moveSetValue);
        ui->boxSetMovVal->setRange(0, 200);
    }
    else
    {
        cout << "Error of units" << endl;
    }
}

void ZaberControlWidget::on_listDevice_itemActivated(QListWidgetItem *item)
{
    ui->grpZaberControl->setEnabled(true);
    fDevIndex = ui->listDevice->currentIndex().row();
    auto dev = gconm->getDeviceList()[fDevIndex];
    monitorValue = dev.getPosition(zaber::motion::Units::LENGTH_MILLIMETRES);
    ProcessMonitorValue();
}

void ZaberControlWidget::on_btnMove_clicked()
{
    if (fDevRunningFlag)
        return;
    auto dev = gconm->getDeviceList()[fDevIndex];
    ui->btnMove->setEnabled(false);

    fMonitorRuningFlag = 1;
    startOneRunning(moveSetValue);

    //    for(int i = 0; i < 10; i++)
    //    {
    //        auto pos = dev.getPosition(zaber::motion::Units::LENGTH_MILLIMETRES);
    //        cout << pos << endl;
    //    }
    //    dev.moveAbsolute(moveSetValue, zaber::motion::Units::LENGTH_MILLIMETRES);
}

void ZaberControlWidget::startOneRunning(double pos)
{
    fProcessingPos = pos;
    fDevRunningFlag = 1;
    fMonitorTimer.start(100);

    emit startRunning(this, 0);
}

void ZaberControlWidget::on_sliderMonitor_actionTriggered(int action)
{
    ui->sliderMonitor->setValue(monitorValue / maxSetValue * maxSliderValue);
}

void ZaberControlWidget::on_RunningStoped()
{
    Sleep(100);
    ui->btnMove->setEnabled(true);
    auto dev = gconm->getDeviceList()[fDevIndex];
    auto pos = dev.getPosition(zaber::motion::Units::LENGTH_MILLIMETRES);
    moveSetValue = monitorValue = pos;

    fMonitorTimer.stop();
    fMonitorRuningFlag = 0;
    fDevRunningFlag = 0;
    ProcessMonitorValue();
    ProcessMoveSetValue();

    // // Emit to other class, fProcessingHandle shold increment here;
    // std::cout << "Test: End of Running: " << fProcessingHandle << '\t' << pos << std::endl;
    // emit moveRequestDone(fProcessingHandle++);

    // // Start Next moving
    // if (fHandles.size() != 0)
    // {
    //     fProcessingHandle = fHandles.front();
    //     startOneRunning(fRequestPos.front());
    //     fHandles.pop();
    //     fRequestPos.pop();
    // }
}

void ZaberControlWidget::ProcessMonitorValue()
{
    int value = monitorValue / maxSetValue * maxSliderValue;
    ui->sliderMonitor->setValue(value);
    ui->lblMonitor->setText(tr(to_string(monitorValue).c_str()));
}

void ZaberControlWidget::ProcessMoveSetValue()
{
    int value = moveSetValue / maxSetValue * maxSliderValue;
    ui->sliderPosition->setValue(value);
    if (ui->boxUnits->currentText() == tr("cm"))
    {
        ui->boxSetMovVal->setValue(moveSetValue / 10);
        ui->boxSetMovVal->setRange(0, 20);
    }
    else if (ui->boxUnits->currentText() == tr("mm"))
    {
        ui->boxSetMovVal->setValue(moveSetValue);
        ui->boxSetMovVal->setRange(0, 200);
    }
    else
    {
        cout << "Error of units" << endl;
    }
}

void ZaberControlWidget::on_btnMonitor_clicked()
{
    auto dev = gconm->getDeviceList()[fDevIndex];
    monitorValue = dev.getPosition(zaber::motion::Units::LENGTH_MILLIMETRES);
    ProcessMonitorValue();
}

// Motion position list generation
#include "General.h"
bool ZaberControlWidget::GeneratePosList()
{
    ui->listPosition->clear();
    std::vector<double> temp;
    bool rtn = UserDefine::ParseLine(ui->linePositionSetting->text().toStdString(), temp);
    if (!rtn)
        return rtn;

    fPosList.clear();
    for (int i = 0; i < temp.size(); i++)
    {
        if (temp[i] >= 0 && temp[i] < 200)
        {
            fPosList.push_back(temp[i]);
        }
    }
    for (int i = 0; i < fPosList.size(); i++)
    {
        QString temp = "Position: " + QString::number(fPosList[i]) + " mm";
        ui->listPosition->addItem(temp);
    }

    return rtn;
}

void ZaberControlWidget::on_btnGenerateList_clicked()
{
    GeneratePosList();
}

void ZaberControlWidget::on_btnClearList_clicked()
{
    ui->listPosition->clear();
    fPosList.clear();
}

bool ZaberControlWidget::GetPositionList(int deviceHandle, double &pos)
{
    if (deviceHandle >= fPosList.size() || deviceHandle < 0)
    {
        pos = 0;
        return false;
    }
    pos = fPosList[deviceHandle];
    return true;
}

bool ZaberControlWidget::JudgeLastMotion(int deviceHandle)
{
    if (deviceHandle == fPosList.size() - 1)
        return true;
    return false;
}

#include <Windows.h>
#include "FEEControlWidget.h"

ZaberTest::ZaberTest(double start, double end, double step) : fOccupied(0)
{
    SetZaberPar(start, end, step);

    connect(gZaberWindow, &ZaberControlWidget::moveRequestStart, this, &ZaberTest::handle_MoveStart);
    connect(gZaberWindow, &ZaberControlWidget::moveRequestDone, this, &ZaberTest::handle_MoveDone);

    connect(gFEEControlWin, &FEEControlWin::startDAQSignal, this, &ZaberTest::handle_DAQStart);
    connect(gFEEControlWin, &FEEControlWin::stopDAQSignal, this, &ZaberTest::handle_DAQDone);
}

bool ZaberTest::SetZaberPar(double start, double end, double step)
{
    if (IsOccupied())
        return false;
    fposStart = start;
    fposEnd = end;
    fstep = step;
    return true;
}

void ZaberTest::handle_MoveDone(int handle)
{
    // Test of Queue
    std::cout << "End Handle: " << gHandle << '\t' << handle << std::endl;

    // Test of continuous control
    double pos = gZaberWindow->getPosition();
    // Sleep(1000);
    // if (pos < 10)
    //     gZaberWindow->handle_MoveRequest(pos + 1);
}

void ZaberTest::handle_MoveStart(int handle)
{
    std::cout << "Start Handle: " << handle << std::endl;
    gHandle = handle;
}

void ZaberTest::startMove()
{
    fOccupied = 1;
    gZaberWindow->handle_MoveRequest(fposStart);

    // Test of Continuous control
    // gZaberWindow->handle_MoveRequest(0);

    // Test of Queue
    // for (int i = 0; i < 10; i++)
    // {
    //     gZaberWindow->handle_MoveRequest(i);
    // }
}

void ZaberTest::handle_DAQStart()
{
}

void ZaberTest::handle_DAQDone()
{
}

void DeviceMove::startMove(ZaberControlWidget *w, bool flagContinuous)
{
    if (!flagContinuous)
    {
        auto dev = gconm->getDeviceList()[gZaberWindow->fDevIndex];
        dev.moveAbsolute(w->fProcessingPos, zaber::motion::Units::LENGTH_MILLIMETRES);
        emit moveReady();
    }
    else
    {
        StartTest();
    }
}

bool DeviceMove::ProcessDeviceHandle(int deviceHandle)
{
    // First Extract DAQ info from gZaberWindow;
    gZaberWindow->GenerateDAQRequestInfo(deviceHandle, fDAQInfo);

    auto dev = gconm->getDeviceList()[gZaberWindow->fDevIndex];
    // dev.moveAbsolute(w->fProcessingPos, zaber::motion::Units::LENGTH_MILLIMETRES);
    double pos = 0;
    auto rtn = gZaberWindow->GetPositionList(deviceHandle, pos);
    dev.moveAbsolute(pos, zaber::motion::Units::LENGTH_MILLIMETRES);

    emit moveReady();
    return rtn;
}

bool DeviceMove::JudgeLastLoop(int deviceHandle)
{
    return gZaberWindow->JudgeLastMotion(deviceHandle);
}

void ZaberControlWidget::on_btnStartConMotion_clicked()
{
    emit startRunning(this, 1);
}

void ZaberControlWidget::StartMonitorClock()
{
    fMonitorTimer.start(100);
}

void ZaberControlWidget::StopMonitorClock()
{
    fMonitorTimer.stop();
}

void ZaberControlWidget::on_btnStopConMotion_clicked()
{
    fMoveWorker->ForceStopDevice();
}

const UserDefine::DAQRequestInfo &ZaberControlWidget::GenerateDAQRequestInfo(int deviceHandle, UserDefine::DAQRequestInfo &daq)
{
    daq.DAQTime = ui->timeDAQSetting->time();
    daq.leastBufferEvent = ui->boxLeastEvents->value();
    daq.msBufferSleep = ui->boxBufferWait->value();
    daq.nDAQCount = ui->boxDAQEvent->value();

    char fileNameTemp[256];
    sprintf(fileNameTemp, "StripTest-Pos-%.2f", fPosList[deviceHandle]);
    daq.sFileName = fileNameTemp;
    ui->lblFileName->setText(QString::fromStdString(daq.sFileName));
    // daq.sFileName = ui->lblFileName->text().toStdString();

    daq.sPath = fsFilePath.toStdString();
    daq.clearQueueFlag = ui->boxClearQueue->isChecked();
    return daq;
}
