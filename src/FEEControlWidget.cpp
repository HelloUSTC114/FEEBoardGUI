#include "FEEControlWidget.h"
#include "./ui_FEEControlWidget.h"

// ROOT
#include "TH1D.h"

// User
#include "ROOTDraw.h"
#include "feecontrol.h"
#include "configfileparser.h"
#include "datamanager.h"

// STL
#include <string>
#include <iostream>

// Platform
#include <windows.h>

// QT
#include <QDir>
#include <QFileDialog>
#include <QDateTime>
#include <QStringList>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QButtonGroup>

using namespace std;

QColor redColor(255, 0, 0), blackColor(0, 0, 0), greenColor(0, 255, 0);

bool DAQRuning::JudgeLoopFlag(FEEControlWin *w, int nEventCount)
{
    // bool timeFlag = (w->fDAQSettingTime == QTime(0, 0, 0, 0)) || (w->fDAQSettingTime >= QTime::fromMSecsSinceStartOfDay(w->fDAQStartTime.msecsTo(QTime::currentTime())));
    // cout << "Flag Judging: " << endl;
    // std::cout << w->fDAQStartTime.toString("yyyy-MM-dd,hh:mm:ss").toStdString() << std::endl;
    // std::cout << w->fDAQSettingTime.msecsSinceStartOfDay() << std::endl;
    // std::cout << w->fDAQStartTime.addMSecs(w->fDAQSettingTime.msecsSinceStartOfDay()).toString("yyyy-MM-dd,hh:mm:ss").toStdString() << std::endl;
    // std::cout << QDateTime::currentDateTime().toString("yyyy-MM-dd,hh:mm:ss").toStdString() << std::endl;
    // std::cout << (w->fDAQStartTime.addMSecs(w->fDAQSettingTime.msecsSinceStartOfDay()) < QDateTime::currentDateTime()) << std::endl;
    // std::cout << timeFlag << std::endl;

    bool timeFlag = (w->fDAQSettingTime == QTime(0, 0, 0, 0)) || (w->fDAQStartTime.addMSecs(w->fDAQSettingTime.msecsSinceStartOfDay()) >= QDateTime::currentDateTime());

    // (w->fDAQSettingTime >= QTime::fromMSecsSinceStartOfDay(w->fDAQStartTime.msecsTo(QTime::currentTime())));

    bool nEventFlag = (w->fDAQSettingCount < 0 || nEventCount < w->fDAQSettingCount);
    bool loopFlag = w->fDAQRuningFlag && nEventFlag && timeFlag;
    return loopFlag;
}

void DAQRuning::startDAQ(FEEControlWin *w)
{
    cout << "Starting DAQ in DAQRuning" << endl;
    cout << w->fDAQRuningFlag << endl;

    int nDAQLoop = 0;
    int nDAQEventCount = 0;
    w->fDAQStartTime = QDateTime::currentDateTime();

    bool loopFlag = JudgeLoopFlag(w, 0);
    for (nDAQLoop = 0; loopFlag; nDAQLoop++)
    {
        gBoard->ReadFifo(w->fDAQBufferSleepms);
        nDAQEventCount += gDataManager->ProcessFEEData(gBoard);
        emit UpdateDAQCount(gDataManager->GetTotalCount());

        loopFlag = JudgeLoopFlag(w, nDAQEventCount);
    }
    emit DAQStopSignal(nDAQLoop);
}

FEEControlWin::FEEControlWin(QWidget *parent)
    : QWidget(parent), ui(new Ui::FEEControlWin)
{
    ui->setupUi(this);

    // FEE control Tab
    ui->btnFileInit->setEnabled(false);
    ui->btnFileClose->setEnabled(false);

    ui->grpDAQctrl->setEnabled(false);
    ui->grpFEEInfo->setEnabled(false);
    ui->grpHVctrl->setEnabled(false);
    ui->grpTMea->setEnabled(false);
    ui->grpTMoni->setEnabled(false);

    // FEE DAQ
    fDAQProcessor = new DAQRuning;
    fDAQProcessor->moveToThread(&fWorkThread3);
    connect(this, &FEEControlWin::startDAQSignal, fDAQProcessor, &DAQRuning::startDAQ);
    connect(fDAQProcessor, &DAQRuning::UpdateDAQCount, this, &FEEControlWin::handle_DAQCount);
    connect(fDAQProcessor, &DAQRuning::DAQStopSignal, this, &FEEControlWin::on_DAQStoped);
    connect(&fWorkThread3, &QThread::finished, fDAQProcessor, &QObject::deleteLater);

    connect(&fDAQClock, &QTimer::timeout, this, &FEEControlWin::update_DAQClock);
    fDAQClock.setTimerType(Qt::VeryCoarseTimer);

    ui->grpDAQctrl->setEnabled(false);
    fWorkThread3.start();

    // FEE CITIROC CONFIG tab
    ui->tabCITIROC->setEnabled(false);

    // FEE Logic
    fpbtngrpLogic = new QButtonGroup(this);
    fpbtngrpLogic->addButton(ui->btnLogic0, 0);
    fpbtngrpLogic->addButton(ui->btnLogic1, 1);
    fpbtngrpLogic->addButton(ui->btnLogic2, 2);
    fpbtngrpLogic->addButton(ui->btnLogic3, 3);
    ui->btnLogic0->setChecked(1);
    ui->btnSendLogic->setEnabled(false);

    // FEE CITIROC configuration
    ui->btnSendConfig->setEnabled(false);

    flblHGAmp = new QLabel(ui->grpChs);
    flblHGAmp->setText(tr("HG Amp"));
    ui->gridChs->addWidget(flblHGAmp, 0, 1, 1, 1);

    flblLGAmp = new QLabel(ui->grpChs);
    flblLGAmp->setText(tr("LG Amp"));
    ui->gridChs->addWidget(flblLGAmp, 0, 2, 1, 1);

    flblBias = new QLabel(ui->grpChs);
    flblBias->setText(tr("Bias"));
    ui->gridChs->addWidget(flblBias, 0, 3, 1, 1);

    flblBiasBox = new QLabel(ui->grpChs);
    flblBiasBox->setText(tr("En Bias"));
    ui->gridChs->addWidget(flblBiasBox, 0, 4, 1, 1);

    flblPABox = new QLabel(ui->grpChs);
    flblPABox->setText(tr("Dis PA"));
    ui->gridChs->addWidget(flblPABox, 0, 5, 1, 1);

    flblChMaskBox = new QLabel(ui->grpChs);
    flblChMaskBox->setText(tr("Mask"));
    ui->gridChs->addWidget(flblChMaskBox, 0, 6, 1, 1);

    for (int i = 0; i < 32; i++)
    {
        // label for channels
        flblChs[i] = new QLabel(ui->grpChs);
        flblChs[i]->setText(tr("Ch%1 ").arg(i));
        flblChs[i]->setAlignment(Qt::AlignHCenter);
        ui->gridChs->addWidget(flblChs[i], i + 1, 0, 1, 1);

        // HG Amp Settings
        fspinsHGAmp[i] = new QSpinBox(ui->grpChs);
        fspinsHGAmp[i]->setObjectName(tr("spinsAmpHG%1").arg(i));
        fspinsHGAmp[i]->setMinimum(0);
        fspinsHGAmp[i]->setMaximum(63);
        fspinsHGAmp[i]->setValue(45);
        //        fspinsHGAmp[i]->
        ui->gridChs->addWidget(fspinsHGAmp[i], i + 1, 1, 1, 1);

        // LG Amp Settings
        fspinsLGAmp[i] = new QSpinBox(ui->grpChs);
        fspinsLGAmp[i]->setObjectName(tr("spinsAmpLG%1").arg(i));
        fspinsLGAmp[i]->setMinimum(0);
        fspinsLGAmp[i]->setMaximum(63);
        fspinsLGAmp[i]->setValue(45);
        ui->gridChs->addWidget(fspinsLGAmp[i], i + 1, 2, 1, 1);

        // bias DAC settings
        fspinsBias[i] = new QSpinBox(ui->grpChs);
        fspinsBias[i]->setObjectName(tr("spinsAmp%1").arg(i));
        fspinsBias[i]->setMinimum(0);
        fspinsBias[i]->setMaximum(255);
        fspinsBias[i]->setValue(100);
        ui->gridChs->addWidget(fspinsBias[i], i + 1, 3, 1, 1);

        // bias DAC checkboxs
        fcbBias[i] = new QCheckBox(ui->grpChs);
        fcbBias[i]->setChecked(true);
        ui->gridChs->addWidget(fcbBias[i], i + 1, 4, 1, 1);
        ui->gridChs->setAlignment(fcbBias[i], Qt::AlignHCenter);

        // PA checkboxs
        fcbDisablePA[i] = new QCheckBox(ui->grpChs);
        fcbDisablePA[i]->setChecked(false);
        ui->gridChs->addWidget(fcbDisablePA[i], i + 1, 5, 1, 1);
        ui->gridChs->setAlignment(fcbDisablePA[i], Qt::AlignHCenter);
        //        ui->gridChs->setAlignment(Qt::AlignCenter);

        // Channel Mask checkboxs
        fcbChannelMask[i] = new QCheckBox(ui->grpChs);
        fcbChannelMask[i]->setChecked(false);
        ui->gridChs->addWidget(fcbChannelMask[i], i + 1, 6, 1, 1);
        ui->gridChs->setAlignment(fcbChannelMask[i], Qt::AlignHCenter);
        gBoard->SetChannelMask(i, 0, fChannelMasks);
    }

    // Fiber Test
    ui->tabFiberTest->setEnabled(false);
    // auto grid1 = new QGridLayout(ui->frmFiberTestDraw);
    // on_btnDraw_clicked();
    // //    grid1->addWidget(fdrawWin, 0, 0, 7, 1);
    // fdrawWin->show();

    // Strip Test
    ui->tabStripTest->setEnabled(false);

    // tabROOTtest
    // auto grid = new QGridLayout(ui->tabStripTest);
    // fdrawWin2 = new PlotWindow(1);
    // grid->addWidget(fdrawWin2, 0, 0, 7, 1);

    // DAQ setting & DAQ Draw
    ui->timeDAQSetting->setTime(QTime(0, 0, 0, 0));
    ui->boxDAQEvent->setValue(-1);
    connect(&fDrawerTimer, SIGNAL(timeout()), this, SLOT(handle_ContinousDraw()));
    connect(fDAQProcessor, &DAQRuning::DAQStopSignal, this, &FEEControlWin::on_btnStopDraw_clicked); // If DAQ stop, than stop Fiber Draw Timer

    // End
    // show();
    ui->tabTotal->setCurrentIndex(0);
    // fdrawWin->Update();
    // fdrawWin2->Update();
}

FEEControlWin *FEEControlWin::Instance()
{
    static FEEControlWin *instance = new FEEControlWin;
    return instance;
}

FEEControlWin::~FEEControlWin()
{
    fWorkThread3.quit();
    fWorkThread3.wait();

    for (int i = 0; i < fWinList.size(); i++)
    {
        delete fWinList[i];
        fWinList[i] = NULL;
    }
    fdrawWin = NULL;
    fWinList.clear();
    // delete fdrawWin2;

    delete ui;
}

// FEE Control
void FEEControlWin::PrintT()
{
    gBoard->ReadTemp();
    double temp[4];
    gBoard->GetTemp(temp);

    ui->lblTOut_0->setText(QString::number(temp[0]));
    ui->lblTOut_1->setText(QString::number(temp[1]));
    ui->lblTOut_2->setText(QString::number(temp[2]));
    ui->lblTOut_3->setText(QString::number(temp[3]));

    // ui->brsMessage->setTextColor(QColor(0, 255, 0));
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append("Temperature Monitor");

    // ui->brsMessage->setTextColor(QColor(0, 0, 0));
    ui->brsMessage->setFontWeight(QFont::Normal);
    for (int i = 0; i < 4; i++)
    {
        ui->brsMessage->append(tr("Group: %1, T: ").arg(i) + QString::number(temp[i]));
    }
}

void FEEControlWin::PrintHV()
{
    gBoard->HVMonitor();
    auto hv = gBoard->GetHV();
    ui->lblVSetOut->setText(QString::number(hv.OV_set));
    ui->lblVMonOut->setText(QString::number(hv.OV_moni));
    ui->lblIMonOut->setText(QString::number(hv.OC_moni));
}

void FEEControlWin::PrintClock()
{
    double freq = gBoard->ReadFreq();
    ui->lblSi570Out->setText(QString::number(freq));

    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append("Checkout Si570 Frequency: ");
    ui->brsMessage->append(QString::number(freq));
    ui->brsMessage->setFontWeight(QFont::Normal);
}

void FEEControlWin::PrintConnection(bool flag)
{
    if (!flag)
    {
        ui->brsMessage->setTextColor(QColor(255, 0, 0));
        ui->brsMessage->setFontWeight(QFont::Bold);
        ui->brsMessage->append("Connect Failed!");

        ui->brsMessage->setTextColor(QColor(0, 0, 0));
        ui->brsMessage->setFontWeight(QFont::Normal);
        return;
    }

    ui->lblIPOut->setText(tr(gBoard->GetIP().c_str()));
    ui->lblPortOut->setText(QString::number(gBoard->GetPort()));
    ui->lblSockOut->setText(QString::number(gBoard->GetSock()));

    ui->brsMessage->setTextColor(QColor(0, 255, 0));
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append("Connect Success!");

    ui->brsMessage->setTextColor(QColor(0, 0, 0));
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append(tr("IP: ") + QString::fromStdString(gBoard->GetIP()));
    ui->brsMessage->append(tr("Port: ") + QString::number(gBoard->GetPort()));
}

void FEEControlWin::ProcessConnect()
{
    ui->btnConnect->setEnabled(false);

    // tab FEE Control
    ui->grpDAQctrl->setEnabled(true);
    ui->grpFEEInfo->setEnabled(true);
    ui->grpHVctrl->setEnabled(true);
    ui->grpTMea->setEnabled(true);
    ui->grpTMoni->setEnabled(true);

    // other tabs
    ui->tabCITIROC->setEnabled(true);
    ui->tabFiberTest->setEnabled(true);
    ui->tabStripTest->setEnabled(true);

    // Init File manager
    if (!gDataManager->IsOpen())
    {
        ui->grpDAQStart->setEnabled(false);
        ui->btnDAQStop->setEnabled(false);
    }
    else
    {
        ui->grpDAQStart->setEnabled(true);
        ui->btnFiberTest->setEnabled(true);

        ui->btnDAQStop->setEnabled(false);
    }

    // Logic module
    ui->btnSendLogic->setEnabled(true);

    // Send Configuration button
    ui->btnSendConfig->setEnabled(true);

    PrintHV();
    PrintClock();
    PrintT();

    // Select Default Logic
    on_btnSendLogic_clicked();
    bool parserFlag = gParser->Init();
    if (parserFlag)
    {
        ui->brsMessage->setTextColor(greenColor);
        ui->brsMessage->setFontWeight(QFont::Bold);
        ui->brsMessage->append(tr("Default CITIROC Config parsed successfully."));
        ui->brsMessage->setTextColor(blackColor);
        ui->brsMessage->setFontWeight(QFont::Normal);

        SendCITIROCConfig();
        PrintToScreen();
    }
    else
    {
        ui->brsMessage->setTextColor(redColor);
        ui->brsMessage->setFontWeight(QFont::Bold);
        ui->brsMessage->append(tr("Error: cannot parse default CITIROC Config file."));
        ui->brsMessage->setTextColor(blackColor);
        ui->brsMessage->setFontWeight(QFont::Normal);
    }
}

void FEEControlWin::on_btnConnect_clicked()
{
    std::string ip = ui->lineIP->text().toStdString();
    int port = ui->boxPort->value();

    ui->brsMessage->setTextColor(QColor(0, 0, 0));
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append("Try to connect: ");
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append(QString::fromStdString(gBoard->GetIP()) + ":" + QString::number(gBoard->GetPort()));

    gBoard->SetIP(ip);
    gBoard->SetFEEPort(port);

    fConnected = gBoard->TestConnect();
    PrintConnection(fConnected);
    if (fConnected)
    {
        ProcessConnect();
    }
}

void FEEControlWin::on_btnPath_clicked()
{
    fsFilePath = QFileDialog::getExistingDirectory(this, tr("Choosing File Path"), QDir::currentPath() + "/../MuonTestControl");

    if (fsFilePath == "")
        return;

    ui->brsMessage->setTextColor(redColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("Change File Path: "));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append(tr("File Path: ") + fsFilePath);

    ui->btnFileInit->setEnabled(true);
}

void FEEControlWin::on_btnHVON_clicked()
{
    gBoard->HVON();
    Sleep(500);
    gBoard->HVMonitor();
    PrintHV();
}

void FEEControlWin::on_btnHPO_clicked()
{
    PrintHV();
}

void FEEControlWin::on_btnHVOFF_clicked()
{
    gBoard->HVOFF();
}

void FEEControlWin::on_btnHVSet_clicked()
{
    gBoard->HVSet(ui->boxHVSet->value());
}

void FEEControlWin::on_btnRegTest_clicked()
{
    gBoard->TestReg();
    PrintClock();
}

void FEEControlWin::on_btnTMon_clicked()
{
    PrintT();
}
// FEE Control

// ROOT File Open, DAQ Control: Start, Stop
bool FEEControlWin::GenerateROOTFile()
{
    fFileTimeStamp = QDateTime::currentDateTime();
    ui->lblFileOut->setText(fsFilePath + "/" + fsFileName + "*.root");
    fsFileNameTotal = fsFileName;
    fsFileNameTotal += fFileTimeStamp.toString("-yyyy-MM-dd-hh-mm-ss");
    fsFileNameTotal += ".root";

    std::cout << fsFileNameTotal.toStdString() << std::endl;

    ui->brsMessage->setTextColor(greenColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("Generating File: "));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append(tr("File Path: ") + fsFilePath);
    ui->brsMessage->append(tr("File Name: ") + fsFileNameTotal);

    bool rtn = gDataManager->Init((fsFilePath + "/" + fsFileNameTotal).toStdString());
    if (!rtn)
    {
        ui->brsMessage->setTextColor(redColor);
        ui->brsMessage->setFontWeight(QFont::Bold);
        ui->brsMessage->append(tr("Generating File Error!"));
    }
    return rtn;
}

void FEEControlWin::on_btnFileInit_clicked()
{
    fsFileName = ui->lblFileName->text();
    auto rtn = GenerateROOTFile();
    if (!rtn)
        return;
    ui->btnFileInit->setEnabled(false);
    ui->btnFileClose->setEnabled(true);

    ui->grpDAQStart->setEnabled(true);
    ui->btnDAQStop->setEnabled(false);
}

void FEEControlWin::on_btnFileClose_clicked()
{
    gDataManager->Close();
    ui->btnFileInit->setEnabled(true);
    ui->btnFileClose->setEnabled(false);

    ui->brsMessage->setTextColor(greenColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("File Closed. "));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append(tr("File Path: ") + fsFilePath);
    ui->brsMessage->append(tr("File Name: ") + fsFileName);

    ui->grpDAQStart->setEnabled(false);

    ui->btnDAQStop->setEnabled(false);
}

void FEEControlWin::handle_DAQCount(int nCount)
{
    // Update real count and time monitor first
    fDAQRealCount = nCount;
    fDAQRealTime = fDAQStartTime.msecsTo(QDateTime::currentDateTime());

    auto sCount = QString::number(nCount);
    ui->lineDAQCount->setText(sCount);

    double countRate = nCount / (double)fDAQRealTime * 1000; // Count rate in unit 1/s
    ui->lineCountRate->setText(QString::number(countRate));

    double percentCount = nCount / (double)fDAQSettingCount * 100;
    double percentTime = fDAQRealTime / (double)fDAQSettingTime.msecsSinceStartOfDay() * 100;

    if (fDAQSettingCount < 0)
    {
        if (fDAQSettingTime == QTime(0, 0, 0, 0))
            ui->pbarDAQ->setValue(0);
        else
            ui->pbarDAQ->setValue(percentTime);
    }
    else
    {
        if (fDAQSettingTime == QTime(0, 0, 0, 0))
            ui->pbarDAQ->setValue(percentCount);
        else
            ui->pbarDAQ->setValue(percentCount > percentTime ? percentCount : percentTime);
    }
}

// Only used to show rough clock
void FEEControlWin::update_DAQClock()
{
    int time = fDAQStartTime.msecsTo(QDateTime::currentDateTime());
    ui->timerDAQ->setTime(QTime::fromMSecsSinceStartOfDay(time));
}

void FEEControlWin::on_btnDAQStart_clicked()
{
    TryStartDAQ(fsFilePath.toStdString(), fsFileName.toStdString(), ui->boxDAQEvent->value(), ui->timeDAQSetting->time(), ui->boxBufferWait->value());
}

bool FEEControlWin::TryStartDAQ(std::string sPath, std::string sFileName, int nDAQCount, QTime DAQTime, int msBufferSleep)
{
    if (fDAQRuningFlag)
        return false;
    if (!fConnected)
    {
        on_btnConnect_clicked();
        if (!fConnected)
            return false;
    }

    QString tempFileName = QString::fromStdString(sFileName);
    ui->lblFileName->setText(tempFileName);
    fsFilePath = QString::fromStdString(sPath);
    fsFileName = QString::fromStdString(sFileName);

    if (gDataManager->IsOpen())
    {
        on_btnFileClose_clicked();
    }

    auto rtn = GenerateROOTFile();
    if (!rtn)
        return false;
    if (!gDataManager->IsOpen())
        return false;

    // Force DAQ start
    ForceStartDAQ(nDAQCount, DAQTime, msBufferSleep);

    // Draw Start
    on_btnStartDraw_clicked();
    gDataManager->Draw(ui->boxDrawCh->value());
    if (fdrawWin)
        fdrawWin->Update();

    return true;
}

void FEEControlWin::ForceStartDAQ(int nCount, QTime daqTime, int msBufferWaiting)
{
    fDAQSettingCount = nCount;
    fDAQSettingTime = daqTime;
    fDAQBufferSleepms = msBufferWaiting;

    ui->boxDAQEvent->setValue(nCount);
    ui->timeDAQSetting->setTime(daqTime);
    ui->boxBufferWait->setValue(fDAQBufferSleepms);

    fDAQRuningFlag = 1;

    QDateTime dateTime(QDateTime::currentDateTime()); // Start Message
    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("DAQ Start: "));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append(dateTime.toString("yyyy-MM-dd  hh:mm:ss"));

    on_btnHVON_clicked();
    Sleep(1000);

    emit startDAQSignal(this);
    fDAQClock.start(1000);

    // GUI Setting
    ui->btnFileInit->setEnabled(false);
    ui->btnFileClose->setEnabled(false);
    ui->btnDAQStop->setEnabled(true);
    ui->grpDAQStart->setEnabled(false);

    ui->btnFiberTest->setEnabled(false);
    ui->btnStopFT->setEnabled(true);
}

void FEEControlWin::on_DAQStoped(int nDAQLoop)
{
    cout << "DAQ Stoped" << endl;
    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("DAQ Stopped"));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append(tr("Total Loop Number:%1").arg(nDAQLoop));

    ui->grpDAQStart->setEnabled(true);
    ui->btnFiberTest->setEnabled(true);

    ui->btnDAQStop->setEnabled(false);
    ui->btnFileClose->setEnabled(true);

    fDAQRuningFlag = 0;

    fDAQClock.stop();
    update_DAQClock();

    on_btnFileClose_clicked();
    on_btnHVOFF_clicked();

    // Tell other class that DAQ is done
    emit stopDAQSignal();
}

void FEEControlWin::on_btnDAQStop_clicked()
{
    fDAQRuningFlag = 0;
    gBoard->SetFifoReadBreak();
}
// DAQ control end

// CITIROC Configuration control
void FEEControlWin::SelectLogic(int logic)
{
    gBoard->logic_select(logic);

    ui->brsMessage->setTextColor(greenColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("Logic Selected: "));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append(tr("logic %1").arg(logic));
}

void FEEControlWin::on_btnToCITIROC_clicked()
{
    ui->tabTotal->setCurrentIndex(1);
}

void FEEControlWin::on_btnCITIROC_Path_clicked()
{
    sCITIROC_Config_Path = QFileDialog::getExistingDirectory(this, tr("Choosing CITIROC Configuration File Path"), QDir::currentPath() + "/../MuonTestControl/Configuration/");
    ui->lineCITIROC_Path->setText(sCITIROC_Config_Path);
    QDir dir(sCITIROC_Config_Path);

    QStringList probeFilter, scFilter;
    probeFilter << "probe*.txt";
    QStringList probeList = dir.entryList(probeFilter, QDir::Files | QDir::Readable, QDir::Name);
    if (!probeList.empty())
    {
        sCITIROC_pb_file = sCITIROC_Config_Path + "/" + probeList.at(0);
        auto pl = sCITIROC_pb_file.split('/');
        auto pbfile = pl.at(pl.size() - 1);
        ui->lineCITIROC_pbFile->setText(pbfile);
    }

    scFilter << "sc*.txt";
    QStringList scList = dir.entryList(scFilter, QDir::Files | QDir::Readable, QDir::Name);
    if (!scList.empty())
    {
        sCITIROC_sc_file = sCITIROC_Config_Path + "/" + scList.at(0);
        auto sl = sCITIROC_sc_file.split('/');
        auto slfile = sl.at(sl.size() - 1);
        ui->lineCITIROC_scFile->setText(slfile);
    }
}

void FEEControlWin::on_btnCITIROC_scFile_clicked()
{
    sCITIROC_sc_file = QFileDialog::getOpenFileName(this, tr("Choosing CITIROC SC File"), sCITIROC_Config_Path, "sc*.txt");
    ui->lineCITIROC_scFile->setText(sCITIROC_sc_file);
    sCITIROC_Config_Path = sCITIROC_sc_file;
    ui->lineCITIROC_Path->setText("");
}

void FEEControlWin::on_btnCITIROC_pbFile_clicked()
{
    sCITIROC_pb_file = QFileDialog::getOpenFileName(this, tr("Choosing CITIROC Probe File"), sCITIROC_Config_Path, "probe*.txt");
    ui->lineCITIROC_pbFile->setText(sCITIROC_pb_file);
    sCITIROC_Config_Path = sCITIROC_pb_file;
    ui->lineCITIROC_Path->setText("");
}

bool FEEControlWin::RSP_CITIROC_configFile(std::string scFile, std::string pbFile)
{
    cout << "scFile: " << scFile << '\t' << "pbFile: " << pbFile << endl;
    auto rtn = gParser->Init(scFile, pbFile);
    if (!rtn)
        return false;

    // Show Configuration file name
    sCITIROC_pb_file = QString::fromStdString(pbFile);
    ui->lineCITIROC_pbFile->setText(sCITIROC_pb_file);
    sCITIROC_sc_file = QString::fromStdString(scFile);
    ui->lineCITIROC_scFile->setText(sCITIROC_sc_file);
    sCITIROC_Config_Path = sCITIROC_pb_file;
    ui->lineCITIROC_Path->setText("");

    // Send Configuration;
    SendCITIROCConfig();
    // Print to Screen();
    PrintToScreen();

    return true;
}

bool FEEControlWin::ReadCITIROC_configFile()
{
    std::string scFile = (sCITIROC_sc_file).toStdString();
    std::string pbFile = (sCITIROC_pb_file).toStdString();
    cout << "scFile: " << scFile << '\t' << "pbFile: " << pbFile << endl;
    cout << "scFile: " << sCITIROC_sc_file.toStdString() << '\t' << "pbFile: " << sCITIROC_pb_file.toStdString() << endl;
    auto rtn = gParser->Init(scFile, pbFile);
    cout << rtn << endl;
    return rtn;
}

void FEEControlWin::on_btnReadCITIROC_clicked()
{
    ReadCITIROC_configFile();
    SendCITIROCConfig();
    PrintToScreen();
}

bool FEEControlWin::PrintToScreen()
{
    if (!gParser->sConfigValidate())
        return false;

    for (int ch = 0; ch < 32; ch++)
    {
        fspinsBias[ch]->setValue(gParser->GetBiasDAC(ch));
        fcbBias[ch]->setChecked(gParser->GetBiasDACSwitch(ch));
        fspinsHGAmp[ch]->setValue(gParser->Get_AMP_HG_DAC(ch));
        fspinsLGAmp[ch]->setValue(gParser->Get_AMP_LG_DAC(ch));
        fcbDisablePA[ch]->setChecked(gParser->Get_PA_Switcher(ch));

        fcbChannelMask[ch]->setChecked(gBoard->GetMask(ch, fChannelMasks));
    }
    ui->boxDiscDAC1->setValue(gParser->GetDiscDAC1());
    ui->boxDiscDAC2->setValue(gParser->GetDiscDAC2());

    return true;
}

bool FEEControlWin::ScanFromScreen()
{
    if (!gParser->sConfigValidate())
        return false;

    for (int ch = 0; ch < 32; ch++)
    {
        gParser->SetBiasDAC(ch, fspinsBias[ch]->value());
        gParser->EnableBiasDAC(ch, fcbBias[ch]->isChecked());
        gParser->Set_AMP_HG_DAC(ch, fspinsHGAmp[ch]->value());
        gParser->Set_AMP_LG_DAC(ch, fspinsLGAmp[ch]->value());
        gParser->DisablePA(ch, fcbDisablePA[ch]->isChecked());

        gBoard->SetChannelMask(ch, fcbChannelMask[ch]->isChecked(), fChannelMasks);
    }
    gParser->SetDiscDAC1(ui->boxDiscDAC1->value());
    gParser->SetDiscDAC2(ui->boxDiscDAC2->value());

    return true;
}

bool FEEControlWin::SendCITIROCConfig()
{
    auto rtn = gBoard->SendConfig(gParser);
    if (rtn < 0 || rtn == EXIT_FAILURE)
    {
        ui->brsMessage->setTextColor(redColor);
        ui->brsMessage->setFontWeight(QFont::Bold);
        ui->brsMessage->append(tr("CITIROC Config sent error! Error code: %1").arg(rtn));
        return false;
    }

    ui->brsMessage->setTextColor(greenColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("CITIROC Config sent successfully."));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
    //    ui->brsMessage->append(tr("logic %1").arg(logic));

    return true;
}

void FEEControlWin::on_btnScanConfig2Mem_clicked()
{
    ScanFromScreen();
}

void FEEControlWin::on_btnPrintToScreen_clicked()
{
    PrintToScreen();
}

void FEEControlWin::on_btnSendLogic_clicked()
{
    SelectLogic(fpbtngrpLogic->checkedId());
}

void FEEControlWin::on_btnSendConfig_clicked()
{
    if (!ScanFromScreen())
        return;
    SendCITIROCConfig();
}

void FEEControlWin::on_btnSaveCITIROC_clicked()
{
    if (!ScanFromScreen())
        return;
    auto time = QDateTime::currentDateTime();
    auto path = QDir::currentPath();
    path += "/../MuonTestControl/Configuration/";
    auto scFile = "scWrite" + time.toString("-MM-dd-hh-mm-ss") + ".txt";
    auto probeFile = "probeWrite" + time.toString("-MM-dd-hh-mm-ss") + ".txt";

    ui->tabTotal->setCurrentIndex(1);
    auto rtn = gParser->PrintConfigFile((path + scFile).toStdString(), (path + probeFile).toStdString());
    if (!rtn)
    {
        ui->brsMessage->setTextColor(redColor);
        ui->brsMessage->setFontWeight(QFont::Bold);
        ui->brsMessage->append(tr("CITIROC Config Write error!"));
    }

    ui->brsMessage->setTextColor(greenColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("CITIROC Config Write successfully."));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append("Path: " + path);
    ui->brsMessage->append("SC File: " + scFile);
    ui->brsMessage->append("Probe File: " + probeFile);
}
// CITIROC Configuration control End

// Fiber Test Control
void FEEControlWin::on_btnFiberTest_clicked()
{
    if (fDAQRuningFlag)
        return;
    if (!fConnected)
    {
        on_btnConnect_clicked();
        if (!fConnected)
            return;
    }
    fsFileName = "Fiber" + QString::number(ui->boxFiberNo->value()) + "-End" + QString::number(ui->boxFiberEnd->value());
    ui->lblFileName->setText(fsFileName);
    if (fsFilePath == "")
    {
        on_btnPath_clicked();
    }

    // If File Dialog quit unexpectedly, return;
    if (fsFilePath == "")
        return;

    // Select Logic
    ui->btnLogic0->setChecked(true);
    on_btnSendLogic_clicked();

    TryStartDAQ(fsFilePath.toStdString(), fsFileName.toStdString(), ui->boxFiberDAQ->value(), ui->timeFiberSetting->time(), 1);
}

void FEEControlWin::on_btnStopFT_clicked()
{
    on_btnDAQStop_clicked();
}
// Fiber Test Control End

//! Draw Control
void FEEControlWin::SetDrawChannel(int channel)
{
    ui->boxDrawCh->setValue(channel);
}

void FEEControlWin::on_btnStartDraw_clicked()
{
    fDrawFlag = 1;
    on_btnDraw_clicked();
    if (!fDAQRuningFlag || !fDrawFlag)
    {
        DrawSingle();
        return;
    }
    else
    {
        fdrawWin->SetOccupied(this);
        fDrawerTimer.setSingleShot(0);
        auto fre = ui->boxRefreshTime->value();
        fDrawerTimer.setTimerType(Qt::VeryCoarseTimer);
        fDrawerTimer.start((fre >= 100) ? fre : 500);
    }
}

void FEEControlWin::DrawSingle()
{
    // static int i = 0;
    // fdrawWin->getROOTWidget()->getCanvas()->cd();
    if (gReadManager->IsOpen() && gReadManager->GetFileName() != (fsFilePath + "/" + fsFileName).toStdString())
        gReadManager->Close();
    if (!gReadManager->IsOpen())
        gReadManager->Init((fsFilePath + "/" + fsFileName).toStdString());
    if (!gReadManager->IsOpen())
        return;
    gReadManager->Draw(ui->boxDrawCh->value());
    fdrawWin->Update();

    // cout << ui->boxDrawCh->value() << '\t' << i++ << endl;
}

void FEEControlWin::on_btnStopDraw_clicked()
{
    fDrawerTimer.stop();
    fDrawFlag = 0;
    if (!fdrawWin)
        return;
    fdrawWin->Update();
    fdrawWin->SetOccupied(false);
}

void FEEControlWin::handle_ContinousDraw()
{
    static int ch = -1;
    if (ch != ui->boxDrawCh->value())
    {
        ch = ui->boxDrawCh->value();
        gDataManager->Draw(ui->boxDrawCh->value());
    }
    if (!fdrawWin->isHidden())
        fdrawWin->Update();
}
// Draw Control End

#include "ftanalyzerwin.h"
void FEEControlWin::on_btnFTAnalyzer_clicked()
{
    // if (!fFTanaWin)
    //     fFTanaWin = new FTAnalyzerWin(this);
    fFTanaWin = new FTAnalyzerWin();
    fFTanaWin->show();
}

#include "zabercontrolwidget.h"
void FEEControlWin::on_btnZaberWindow_clicked()
{
    gZaberWindow->show();
}

void FEEControlWin::on_btnDraw_clicked()
{
    static int winid = 0;
    // if (!fdrawWin || fdrawWin->isHidden())
    if (fdrawWin && !fdrawWin->isHidden())
    {
        fdrawWin->activateWindow();
        return;
    }
    if (fdrawWin && fdrawWin->isHidden())
    {
        // fdrawWin = new PlotWindow(*fdrawWin);
        fdrawWin = new ROOTDraw(*fdrawWin);
        // delete fWinList[0];
        fWinList[0] = fdrawWin;
    }
    if (!fdrawWin)
    {
        // fdrawWin = new PlotWindow(winid++);
        fdrawWin = new ROOTDraw(winid++);
        fWinList.push_back(fdrawWin);
        // fdrawWin->show();
        // fdrawWin->Update();
        // return;
    }

    // fWinList.push_back(fdrawWin);
    fdrawWin->show();
    // fdrawWin->Update();
    fdrawWin->Update();
}

void FEEControlWin::on_btnFiberTab_clicked()
{
    ui->tabTotal->setCurrentIndex(0);
}

// Strip Test Control

#include <fstream>
void FEEControlWin::TryStartStripTestDAQ(int nCount, QTime daqTime, int msBufferWaiting)
{
    ui->boxStripDAQ->setValue(nCount);
    ui->timeStripSetting->setTime(daqTime);
    ui->boxBufferWait->setValue(msBufferWaiting);
    on_btnStripTest_clicked();

    std::ofstream fout(fsStripFileName, ios::app);
    fout << "Start Test: Position: " << fposNow << " Time Stamp: " << fFileTimeStamp.toString("yyyy-MM-dd, hh:mm:ss").toStdString() << std::endl;
    fout.close();
}

#include <fstream>
void FEEControlWin::on_btnStripTest_clicked()
{
    if (fDAQRuningFlag)
        return;
    if (!fConnected)
    {
        on_btnConnect_clicked();
        if (!fConnected)
            return;
    }
    fposNow = 0;
    if (gZaberWindow->isValid())
    {
        fposNow = gZaberWindow->getPosition();
    }

    int stripNo = ui->boxStripNo->value();
    int stripEndNo = ui->boxStripEnd->value();
    fsFileName = "Strip" + QString::number(stripNo) + "-End" + QString::number(stripEndNo) + "-Pos" + QString::number(fposNow);
    ui->lblFileName->setText(fsFileName);
    if (fsFilePath == "")
    {
        on_btnPath_clicked();
    }

    // If File Dialog quit unexpectedly, return;
    if (fsFilePath == "")
        return;

    // Select Logic
    ui->btnLogic3->setChecked(true);
    on_btnSendLogic_clicked();

    TryStartDAQ(fsFilePath.toStdString(), fsFileName.toStdString(), ui->boxStripDAQ->value(), ui->timeStripSetting->time(), 1);
}

void FEEControlWin::on_btnStopST_clicked()
{
    on_btnDAQStop_clicked();
}

void FEEControlWin::GetZaberCtrInfo(double &start, double &end, double &step)
{
    fposStart = ui->boxStartPos->value();
    fposEnd = ui->boxEndPos->value();
    fstep = ui->boxZaberStep->value();

    start = fposStart;
    end = fposEnd;
    step = fstep;
}

void FEEControlWin::SetZaberCtrInfo(double start, double end, double step)
{
    fposStart = start;
    fposEnd = end;
    fstep = step;
    ui->boxStartPos->setValue(fposStart);
    ui->boxEndPos->setValue(fposEnd);
    ui->boxZaberStep->setValue(fstep);
}

const double LEFT = 10;  // Left add in cm
const double RIGHT = 10; // RIGHT add in cm
const double STEP = 0.5; // Step

void FEEControlWin::on_btnZaberPos_clicked()
{
    if (!gZaberWindow->isValid())
        return;
    double posInit = gZaberWindow->getPosition();
    ui->lblZaberPos->setText(QString::number(posInit));

    fposStart = posInit - LEFT;
    fposEnd = posInit + RIGHT;
    fstep = STEP;
    // SetZaberCtrInfo(fposStart, fposEnd, fstep);
    // John Test: Debug
    SetZaberCtrInfo(65, 75, 1);
    gZaberDrawControl->SetZaberPar(fposStart, fposEnd, fstep);
}

void FEEControlWin::on_btnSTStart_clicked()
{
    // output test information for this batch
    if (fsFilePath == "")
        on_btnPath_clicked();
    if (fsFilePath == "")
        return;
    fsStripFileName = (std::string) "ST-Strip" + std::to_string(ui->boxStripNo->value()) + ".txt";
    fsStripFileName = fsFilePath.toStdString() + "/" + fsStripFileName;
    std::cout << fsStripFileName << std::endl;
    ofstream fout(fsStripFileName, ios::app);
    fout << "Strip Batch Test Information" << std::endl;
    fout << "Start Test: Position: " << fposNow;
    fout.close();

    if (!gZaberWindow->isValid())
        return;
    double a, b, c;
    GetZaberCtrInfo(a, b, c);
    gZaberDrawControl->SetZaberPar(fposStart, fposEnd, fstep);
    gZaberDrawControl->startMove();
}

void FEEControlWin::on_btnSTStop_clicked()
{
    gZaberDrawControl->SetBreak();
    on_btnDAQStop_clicked();
}
// Strip Test Control End

ZaberDrawControl::ZaberDrawControl() : fOccupied(0)
{
}

void ZaberDrawControl::ConnectSlots()
{
    connect(gZaberWindow, &ZaberControlWidget::moveRequestStart, this, &ZaberDrawControl::handle_MoveStart);
    connect(gZaberWindow, &ZaberControlWidget::moveRequestDone, this, &ZaberDrawControl::handle_MoveDone);

    connect(gFEEControlWin, &FEEControlWin::startDAQSignal, this, &ZaberDrawControl::handle_DAQStart);
    connect(gFEEControlWin, &FEEControlWin::stopDAQSignal, this, &ZaberDrawControl::handle_DAQDone);
}

void ZaberDrawControl::DisconnectSlots()
{
    disconnect(gZaberWindow, &ZaberControlWidget::moveRequestStart, this, &ZaberDrawControl::handle_MoveStart);
    disconnect(gZaberWindow, &ZaberControlWidget::moveRequestDone, this, &ZaberDrawControl::handle_MoveDone);

    disconnect(gFEEControlWin, &FEEControlWin::startDAQSignal, this, &ZaberDrawControl::handle_DAQStart);
    disconnect(gFEEControlWin, &FEEControlWin::stopDAQSignal, this, &ZaberDrawControl::handle_DAQDone);
}

ZaberDrawControl *ZaberDrawControl::Instance()
{
    static ZaberDrawControl *instance = new ZaberDrawControl;
    return instance;
}

bool ZaberDrawControl::SetZaberPar(double start, double end, double step)
{
    if (IsOccupied())
        return false;
    fposStart = start;
    fposEnd = end;
    fstep = step;
    return true;
}

void ZaberDrawControl::startMove()
{
    fOccupied = 1;
    ConnectSlots();
    gZaberWindow->handle_MoveRequest(fposStart);
    fposSettingNow = fposStart;

    // Test of Continuous control
    // gZaberWindow->handle_MoveRequest(0);

    // Test of Queue
    // for (int i = 0; i < 10; i++)
    // {
    //     gZaberWindow->handle_MoveRequest(i);
    // }
}

void ZaberDrawControl::handle_MoveDone(int handle)
{
    // Test of Queue
    std::cout << "End Handle: " << gHandle << '\t' << handle << std::endl;

    // Test of continuous control
    double pos = gZaberWindow->getPosition();

    gFEEControlWin->TryStartStripTestDAQ(-1, gFEEControlWin->ui->timeStripSetting->time(), 1);

    // Sleep(1000);
    // if (pos < 10)
    //     gZaberWindow->handle_MoveRequest(pos + 1);
}

void ZaberDrawControl::handle_MoveStart(int handle)
{
    std::cout << "Zaber Motion Start Handle: " << handle << std::endl;
    gHandle = handle;
}

void ZaberDrawControl::handle_DAQStart()
{
    std::cout << "Strip Test DAQ Start: " << QTime(0, 3, 0, 0).toString("mm-ss").toStdString() << std::endl;
}

void ZaberDrawControl::handle_DAQDone()
{
    double pos = gZaberWindow->getPosition();

    ofstream fout(gFEEControlWin->GetStripTestInfoFile(), ios::app);
    fout << "End Test: Count Rate: " << gFEEControlWin->GetCountRate() << std::endl;
    fout.close();

    if (pos < fposEnd && !fBreakFlag)
    {
        fposSettingNow += fstep;
        gZaberWindow->handle_MoveRequest(fposSettingNow);
    }
    else
    {
        fOccupied = 0;
        fBreakFlag = 0;
        DisconnectSlots();
    }
}

void FEEControlWin::on_btnMask_clicked()
{
    ScanFromScreen();
    gBoard->send_ch_masks(fChannelMasks);

    ui->brsMessage->setTextColor(QColor(0, 255, 0));
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append("Sent Masks: "+QString::number(fChannelMasks));
}

