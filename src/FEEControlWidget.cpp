#include "FEEControlWidget.h"
#include "./ui_FEEControlWidget.h"

// User
#include "feecontrol.h"
#include "configfileparser.h"
#include "datamanager.h"
#include "ROOTDraw.h"

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
    bool connectionFlag = w->IsConnected();
    bool loopFlag = w->fDAQRuningFlag && nEventFlag && timeFlag && connectionFlag;
    return loopFlag;
}

void DAQRuning::startDAQ(FEEControlWin *w)
{
    // cout << "Test: Starting DAQ in DAQRuning" << endl;
    // cout << w->fDAQRuningFlag << endl;

    int nDAQLoop = 0;
    int nDAQEventCount = 0;
    w->fDAQStartTime = QDateTime::currentDateTime();

    // Clear queue before DAQ Start
    if (w->fFlagClearQueue)
    {
        gBoard->clean_queue(0);
        gBoard->clean_queue(1);
        gBoard->clean_queue(2);
        gBoard->clean_queue(3);
    }

    bool loopFlag = JudgeLoopFlag(w, 0);
    for (nDAQLoop = 0; loopFlag; nDAQLoop++)
    {
        // clock_t test1 = clock();
        auto rtnRead = gBoard->ReadFifo(w->fDAQBufferSleepms, w->fDAQBufferLeastEvents);
        // clock_t test2 = clock();
        if (!rtnRead)
            break;
        nDAQEventCount += gDataManager->ProcessFEEData(gBoard);
        // clock_t test3 = clock();

        emit UpdateDAQCount(gDataManager->GetHGTotalCount());

        loopFlag = JudgeLoopFlag(w, nDAQEventCount);
        // std::cout << "Clocks: " << test2 - test1 << '\t' << test3 - test2 << std::endl;
    }
    emit DAQStopSignal(nDAQLoop);
}

FEEControlWin::FEEControlWin(QWidget *parent)
    : QWidget(parent), ui(new Ui::FEEControlWin)
{
    ui->setupUi(this);

#ifdef USE_FEE_CONTROL_MONITOR
    // Connection Process
    connect(gFEEMonitor, &QtUserConnectionMonitor::connectionBroken, this, &FEEControlWin::handle_connectionBroken);
#endif

    // FEE control Tab
    ui->btnExit->setEnabled(false);

    ui->lineIP->setEnabled(false);
    ui->boxPort->setEnabled(false);

    ui->grpFEEInfo->setEnabled(false);
    ui->grpHVctrl->setEnabled(false);
    ui->grpTMea->setEnabled(false);
    ui->grpTMoni->setEnabled(false);

    // Count Rate monitor
    fCRClock.callOnTimeout(this, &FEEControlWin::RetrieveCountOnce);

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
        gBoard->GenerateChMask(i, 0, fChannelMasks);
    }

    // FEE Masks control
    ConnectMasks();

    // DAQ setting & DAQ Draw
    ui->timeDAQSetting->setTime(QTime(0, 0, 0, 0));
    ui->boxDAQEvent->setValue(-1);
    connect(&fDrawerTimer, SIGNAL(timeout()), this, SLOT(handle_ContinousDraw()));
    connect(fDAQProcessor, &DAQRuning::DAQStopSignal, this, &FEEControlWin::on_btnStopDraw_clicked); // If DAQ stop, than stop Fiber Draw Timer

    // DAQ Draw
    fpbtngrpDrawOption = new QButtonGroup(this);
    fpbtngrpDrawOption->addButton(ui->btnHGDraw, 0);
    fpbtngrpDrawOption->addButton(ui->btnLGDraw, 1);
    fpbtngrpDrawOption->addButton(ui->btnTDCDraw, 2);
    fpbtngrpDrawOption->button(0)->setChecked(1);

    connect(fpbtngrpDrawOption, SIGNAL(buttonClicked(int)), this, SLOT(handle_DrawOption_Changed()));
    connect(ui->boxDrawCh, SIGNAL(textChanged(QString)), this, SLOT(handle_DrawOption_Changed()));

    // Timer Control
    connect(&fDAQControlTimer, SIGNAL(timeout()), this, SLOT(handle_LoopTimer()));
    connect(&fDAQCountDownTimer, SIGNAL(timeout()), this, SLOT(handle_CountDownTimer()));

    // End
    // show();
    ui->tabTotal->setCurrentIndex(0);
    // fdrawWin->Update();
    // fdrawWin2->Update();
}

void FEEControlWin::Test()
{
    std::cout << "Draw option checked id: " << fpbtngrpDrawOption->checkedId() << std::endl;
    fpbtngrpDrawOption->button(2)->setChecked(1);
    std::cout << fpbtngrpDrawOption->checkedButton() << std::endl;
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

    // Stop all clocks
    fOnceTimer.stop();
    fDAQClock.stop();
    fDrawerTimer.stop();
    fCRClock.stop();

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
    ui->lblBoardOut->setText(QString::number(fCurrentBoardNo));

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
    ui->btnExit->setEnabled(true);
    ui->lblLED->setStyleSheet("background-color:rgb(0,255,0)");

    // DAQ Control
    ui->grpDAQctrl->setEnabled(true);

    // Count Rate monitor
    fCRClock.start(1000);

    // tab FEE Control
    ui->grpFEEInfo->setEnabled(true);
    ui->grpHVctrl->setEnabled(true);
    ui->grpTMea->setEnabled(true);
    ui->grpTMoni->setEnabled(true);

    // other tabs
    ui->tabCITIROC->setEnabled(true);

    // Init File manager
    if (!gDataManager->IsOpen())
    {
        // ui->grpDAQStart->setEnabled(false);
        ui->grpDAQStart->setEnabled(true);
        ui->btnDAQStop->setEnabled(false);
    }
    else
    {
        ui->grpDAQStart->setEnabled(true);
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
    SelectLogic(1);
    on_btnAllSetMask_clicked();
    on_btnSendLogic_clicked();
    on_btnMask_clicked();
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

void FEEControlWin::ProcessDisconnect()
{
    ui->lblLED->setStyleSheet("background-color:rgb(255,0,0)");
    fConnected = false;
    // Stop all clocks
    fOnceTimer.stop();
    fDAQClock.stop();
    fDrawerTimer.stop();
    fCRClock.stop();

    ui->btnConnect->setEnabled(true);
    ui->btnExit->setEnabled(false);

    // DAQ Control
    ui->grpDAQctrl->setEnabled(false);

    // tab FEE Control
    ui->grpFEEInfo->setEnabled(false);
    ui->grpHVctrl->setEnabled(false);
    ui->grpTMea->setEnabled(false);
    ui->grpTMoni->setEnabled(false);

    // other tabs
    ui->tabCITIROC->setEnabled(false);

    // Init File manager
    if (!gDataManager->IsOpen())
    {
        // ui->grpDAQStart->setEnabled(false);
        ui->grpDAQStart->setEnabled(false);
        ui->btnDAQStop->setEnabled(false);
    }
    else
    {
        ui->grpDAQStart->setEnabled(false);
        ui->btnDAQStop->setEnabled(false);
    }

    // Logic module
    ui->btnSendLogic->setEnabled(false);

    // Send Configuration button
    ui->btnSendConfig->setEnabled(false);
}

void FEEControlWin::handle_connectionBroken(int boardNo)
{
    if (boardNo == fCurrentBoardNo)
        ProcessDisconnect();
}

void FEEControlWin::on_btnConnect_clicked()
{
    // on_btnGenerateIP_clicked();
    // std::string ip = ui->lineIP->text().toStdString();
    // int port = ui->boxPort->value();
    // gBoard->InitPort(ip, port);

    fCurrentBoardNo = ui->boxBoardNo->value();
    gBoard->InitPort(fCurrentBoardNo);
    ui->brsMessage->setTextColor(QColor(0, 0, 0));
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append("Try to connect: ");
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append(QString::fromStdString(gBoard->GetIP()) + ":" + QString::number(gBoard->GetPort()));

    fConnected = gBoard->TestConnect();
    PrintConnection(fConnected);
    if (fConnected)
    {
        ProcessConnect();
    }
}

void FEEControlWin::ClearList()
{
    ui->listGain->clear();
    ui->listBias->clear();
    ui->listCh->clear();

    fGainList.clear();
    fBiasList.clear();
    fChList.clear();
}

#include "General.h"
void FEEControlWin::GenerateGainList()
{
    ui->listGain->clear();
    std::vector<double> temp;
    bool rtn = UserDefine::ParseLine(ui->lineCITIROCGain->text().toStdString(), temp);
    if (!rtn)
    {
        fGainList.clear();
        fGainList.push_back(fDefaultGain);
    }

    fGainList.clear();
    for (int i = 0; i < temp.size(); i++)
    {
        if (temp[i] >= 0 && temp[i] < 64)
        {
            fGainList.push_back(temp[i]);
        }
    }
    for (int i = 0; i < fGainList.size(); i++)
    {
        QString temp = QString::number(fGainList[i]);
        ui->listGain->addItem(temp);
    }
    return;
}

void FEEControlWin::GenerateBiasList()
{
    ui->listBias->clear();
    std::vector<double> temp;
    bool rtn = UserDefine::ParseLine(ui->lineCITIROCBias->text().toStdString(), temp);
    if (!rtn)
    {
        fBiasList.clear();
        fBiasList.push_back(fDefaultBias);
    }

    fBiasList.clear();
    for (int i = 0; i < temp.size(); i++)
    {
        if (temp[i] >= 0 && temp[i] < 256)
        {
            fBiasList.push_back(temp[i]);
        }
    }
    for (int i = 0; i < fBiasList.size(); i++)
    {
        QString temp = QString::number(fBiasList[i]);
        ui->listBias->addItem(temp);
    }
    return;
}

void FEEControlWin::GenerateChList()
{
    ui->listCh->clear();
    std::vector<double> temp;
    bool rtn = UserDefine::ParseLine(ui->lineSelectChannel->text().toStdString(), temp);
    if (!rtn)
        return;

    fChList.clear();
    for (int i = 0; i < temp.size(); i++)
    {
        if (temp[i] >= 0 && temp[i] < 32)
        {
            fChList.push_back(temp[i]);
        }
    }

    for (int i = 0; i < fChList.size(); i++)
    {
        QString temp = "Channel: " + QString::number(fChList[i]);
        ui->listCh->addItem(temp);
    }

    return;
}

void FEEControlWin::GenerateLoopOrder()
{
    int idxGain = ui->cbbGain->currentIndex();
    int idxBias = ui->cbbBias->currentIndex();
    int idxCh = ui->cbbCh->currentIndex();
    if (ui->cbbCh->currentIndex() == 3)
    {
        fChAttendLoop = 0;
        if (idxGain <= idxBias)
        {
            fGainLoopOrder = 1;
            fBiasLoopOrder = 2;
        }
        else
        {
            fGainLoopOrder = 2;
            fBiasLoopOrder = 1;
        }
        fChLoopOrder = 0;
        ui->cbbGain->setCurrentIndex(fLoopOrder[0] - 1);
        ui->cbbBias->setCurrentIndex(fLoopOrder[1] - 1);
        QString temp = "Channels do not change, change [Gain,Bias] as: [" + QString::number(fGainLoopOrder) + "," + QString::number(fBiasLoopOrder) + "].";
        ui->lblLoopOrder->setText(temp);
    }
    else
    {
        fChAttendLoop = 1;

        std::vector<std::pair<int, int>> tempOrderArray;
        tempOrderArray.push_back(std::pair<int, int>(0, idxGain));
        tempOrderArray.push_back(std::pair<int, int>(1, idxBias));
        tempOrderArray.push_back(std::pair<int, int>(2, idxCh));
        std::sort(tempOrderArray.begin(), tempOrderArray.end(), [](const std::pair<int, int> &temp1, const std::pair<int, int> &temp2)
                  { return temp1.second < temp2.second; });
        for (int i = 0; i < 3; i++)
        {
            int loopIdx = tempOrderArray[i].first;
            // std::cout << loopIdx << '\t' << tempOrderArray[i].second << '\t' << i + 1 << std::endl;
            fLoopOrder[loopIdx] = i + 1;
        }
        ui->cbbGain->setCurrentIndex(fLoopOrder[0] - 1);
        ui->cbbBias->setCurrentIndex(fLoopOrder[1] - 1);
        ui->cbbCh->setCurrentIndex(fLoopOrder[2] - 1);

        QString temp = "Change [Gain,Bias,Channel] as: [" + QString::number(fGainLoopOrder) + "," + QString::number(fBiasLoopOrder) + "," + QString::number(fChLoopOrder) + "].";
        ui->lblLoopOrder->setText(temp);
    }

    for (int i = 0; i < 3; i++)
    {
        fLoopOrderMap[fLoopOrder[i]] = &(fList[i]); // fLoopOrder & fList are combined
    }
    fLoopOrderGeneratedFlag = 1;

    // Clear Loop Counter display
    on_btnClearCounter_clicked();
}

void FEEControlWin::ParseDAQCount(int daqCount, int &idxGain, int &idxBias, int &idxCh)
{
    if (!fLoopOrderGeneratedFlag)
        return;
    if (!fChAttendLoop)
    {
        fCurrentIndexMap[0] = -1;
        // int firstLoopTotal = fLoopOrderMap[1]->size();
        int firstLoopTotal = fLoopOrderMap[1]->size();
        int secondLoopTotal = fLoopOrderMap[2]->size();
        fCurrentIndexMap[1] = daqCount % firstLoopTotal;
        fCurrentIndexMap[2] = (daqCount / firstLoopTotal) % secondLoopTotal;
    }
    else
    {
        int firstLoopTotal = fLoopOrderMap[1]->size();
        int secondLoopTotal = fLoopOrderMap[2]->size();
        int thirdLoopTotal = fLoopOrderMap[3]->size();
        fCurrentIndexMap[1] = daqCount % firstLoopTotal;
        fCurrentIndexMap[2] = (daqCount / firstLoopTotal) % secondLoopTotal;
        fCurrentIndexMap[3] = (daqCount / (firstLoopTotal * secondLoopTotal)) % thirdLoopTotal;
    }
    idxGain = fCurrentIndexMap[fLoopOrder[0]];
    idxBias = fCurrentIndexMap[fLoopOrder[1]];
    idxCh = fCurrentIndexMap[fLoopOrder[2]];

    ui->listGain->setCurrentRow(idxGain);
    ui->listBias->setCurrentRow(idxBias);
    ui->listCh->setCurrentRow(idxCh);
}

void FEEControlWin::UpdateCountDown()
{
    QTime time;
    time = QTime::fromMSecsSinceStartOfDay(fDAQControlTimer.remainingTime());
    ui->lcdCountDown->display(time.toString("HH:mm:ss"));
}

void FEEControlWin::StartDAQInLoop()
{
    ui->lcdLoopCounter->display(fLoopCounter);
    on_btnTMon_clicked();
    // std::cout << "John: Only for Test: Simulation: start DAQ." << std::endl;

    // Change CITIROC Setting
    int idxGain, idxBias, idxCh;
    ParseDAQCount(fLoopCounter, idxGain, idxBias, idxCh);

    if (!fChAttendLoop)
    {
        std::vector<std::pair<int, int>> vBias;
        std::vector<std::pair<int, int>> vGain;

        for (int i = 0; i < fChList.size(); i++)
        {
            vBias.push_back(std::pair<int, int>(fChList[i], fBiasList[idxBias]));
            vGain.push_back(std::pair<int, int>(fChList[i], fGainList[idxGain]));
        }
        Modify_SP_CITIROC_BiasDAC(vBias);
        Modify_SP_CITIROC_HGAmp(vGain);
        Modify_SP_CITIROC_LGAmp(vGain);
    }
    else
    {
        Modify_SP_CITIROC_BiasDAC(fChList[idxCh], fBiasList[idxBias]);
        Modify_SP_CITIROC_HGAmp(fChList[idxCh], fGainList[idxBias]);
        Modify_SP_CITIROC_LGAmp(fChList[idxCh], fGainList[idxBias]);
    }

    // Start DAQ
//     on_btnDAQStart_clicked();
     QTimer::singleShot(15*1000, this, SLOT(on_btnDAQStart_clicked()));

    fLoopCounter++;
}

void FEEControlWin::on_btnHVON_clicked()
{
    gBoard->HVON();
    QTimer::singleShot(2000, this, SLOT(on_btnHPO_clicked()));
}

void FEEControlWin::on_btnHPO_clicked()
{
    PrintHV();
}

void FEEControlWin::on_btnHVOFF_clicked()
{
    gBoard->HVOFF();
    QTimer::singleShot(2000, this, SLOT(on_btnHPO_clicked()));
}

void FEEControlWin::on_btnHVSet_clicked()
{
    gBoard->HVSet(ui->boxHVSet->value());
    QTimer::singleShot(2000, this, SLOT(on_btnHPO_clicked()));
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

void FEEControlWin::on_btnGenerateIP_clicked()
{
    string ip;
    int port;
    FEEControl::GenerateIP(ui->boxBoardNo->value(), ip, port);
    ui->boxPort->setValue(port);
    ui->lineIP->setText(QString::fromStdString(ip));
}

void FEEControlWin::on_btnExit_clicked()
{
    ProcessDisconnect();
    gBoard->BoardExit();

    ui->brsMessage->setTextColor(greenColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("Board Exited."));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
}
// FEE Control END

// Count Rate Monitor
void FEEControlWin::RetrieveCountOnce()
{
    // Update real count and live count
    static QDateTime sLastTestTime = QDateTime::currentDateTime();
    static long sLastLiveCount = -1;
    static long sLastRealCount = -1;
    QDateTime testTime = QDateTime::currentDateTime();
    double msToLastTest = (double)sLastTestTime.msecsTo(testTime);

    uint32_t realCount = 0;
    uint32_t liveCount = 0;
    gBoard->get_real_counter(realCount);
    gBoard->get_live_counter(liveCount);

    ui->lineRealCount->setText(QString::number(realCount));
    ui->lineLiveCount->setText(QString::number(liveCount));
    if (sLastRealCount > 0)
    {
        ui->lineRealCR->setText(QString::number((realCount - sLastRealCount) * 1000.0 / msToLastTest));
    }
    if (sLastLiveCount > 0)
    {
        ui->lineLiveCR->setText(QString::number((liveCount - sLastLiveCount) * 1000.0 / msToLastTest));
    }
    sLastRealCount = realCount;
    sLastLiveCount = liveCount;
    sLastTestTime = testTime;
}

// ROOT File Open, DAQ Control: Start, Stop
void FEEControlWin::on_btnPath_clicked()
{
    fsFilePath = QFileDialog::getExistingDirectory(this, tr("Choosing File Path"), QDir::currentPath() + "/../MuonTestControl/Data");

    if (fsFilePath == "")
        return;

    ui->brsMessage->setTextColor(redColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("Change File Path: "));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append(tr("File Path: ") + fsFilePath);

    // ui->btnFileInit->setEnabled(true);
    ui->btnFileInit->setEnabled(false);
}

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
    gDataManager->SetBoardNo(gBoard->GetBoardNo());
    gDataManager->SetCITIROCConfig(gParser->GetString());
    gDataManager->SetDAQDatime(fFileTimeStamp);
    gDataManager->SetSelectedLogic(gBoard->GetLastLogic());

    gBoard->ReadTemp();
    double temp[4];
    gBoard->GetTemp(temp);
    gDataManager->SetDAQTemp(temp);

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

void FEEControlWin::CloseSaveFile()
{
    gDataManager->Close();
    // ui->btnFileInit->setEnabled(true);
    ui->btnFileClose->setEnabled(false);

    ui->brsMessage->setTextColor(greenColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("File Closed. "));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append(tr("File Path: ") + fsFilePath);
    ui->brsMessage->append(tr("File Name: ") + fsFileName);

    // ui->grpDAQStart->setEnabled(false);

    ui->btnDAQStop->setEnabled(false);
}

void FEEControlWin::on_btnFileClose_clicked()
{
    CloseSaveFile();
    QFile fileTemp(fsFilePath + "/" + fsFileNameTotal);
    fileTemp.remove();
}

//! TODO: Add HG, LG, TDC data counter monitor in GUI
void FEEControlWin::handle_DAQCount(int nCount)
{
    // Update Transist count rate
    static QDateTime sLastTime = QDateTime::currentDateTime();
    static int sLastCount = nCount;
    static double sTransCR = 0;
    static int sTransTimeInteval = 0;
    int transCount = nCount - sLastCount;
    // if ((sTransTimeInteval = sLastTime.msecsTo(QDateTime::currentDateTime())) > 100 && transCount > 0)
    if ((sTransTimeInteval = sLastTime.msecsTo(QDateTime::currentDateTime())) >= 1)
    {
        // std::cout << "Count: " << nCount << '\t' << transCount << '\t' << sTransTimeInteval << std::endl;
        sTransCR = (double)transCount / sTransTimeInteval * 1000;
        sLastCount = nCount;
        sTransTimeInteval = 0;
        sLastTime = QDateTime::currentDateTime();
    }

    ui->lineCountRateTran->setText(QString::number(sTransCR));

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
            ui->pbarDAQ->setValue(percentTime > 100 ? 100 : percentTime);
    }
    else
    {
        if (fDAQSettingTime == QTime(0, 0, 0, 0))
            ui->pbarDAQ->setValue(percentCount > 100 ? 100 : percentCount);
        else
            ui->pbarDAQ->setValue(percentCount > percentTime ? percentCount : percentTime);
    }
}

// Only used to show rough clock
void FEEControlWin::update_DAQClock()
{
    int time = fDAQStartTime.msecsTo(QDateTime::currentDateTime());
    ui->timerDAQ->setTime(QTime::fromMSecsSinceStartOfDay(time));

    // Get count from DataManager
    ui->lineHGDAQCount->setText(QString::number(gDataManager->GetHGTotalCount()));
    ui->lineLGDAQCount->setText(QString::number(gDataManager->GetLGTotalCount()));
    ui->lineTDCDAQCount->setText(QString::number(gDataManager->GetTDCTotalCount()));

    // Get queue length from FEEControl
    // ui->lineHGQueue->setText(QString::number(gBoard->GetHGQueueMonitor()));
    // ui->lineLGQueue->setText(QString::number(gBoard->GetLGQueueMonitor()));
    // ui->lineTDCQueue->setText(QString::number(gBoard->GetTDCQueueMonitor()));
    ui->lineUnreadCount->setText(QString::number(gBoard->GetQueueGroupMonitor() * 20));
}

void FEEControlWin::on_btnDAQStart_clicked()
{
    fsFileName = ui->lblFileName->text();
    TryStartDAQ(fsFilePath.toStdString(), fsFileName.toStdString(), ui->boxDAQEvent->value(), ui->timeDAQSetting->time(), ui->boxBufferWait->value(), ui->boxLeastEvents->value(), ui->boxClearQueue->isChecked());
}

bool FEEControlWin::TryStartDAQ(std::string sPath, std::string sFileName, int nDAQCount, QTime DAQTime, int msBufferSleep, int leastBufferEvent, bool clearBeforeDAQ)
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
        // on_btnFileClose_clicked();
        CloseSaveFile();
    }

    auto rtn = GenerateROOTFile();
    if (!rtn)
        return false;
    if (!gDataManager->IsOpen())
        return false;

    // Force DAQ start
    ForceStartDAQ(nDAQCount, DAQTime, msBufferSleep, leastBufferEvent, clearBeforeDAQ);

    // Draw Start
    on_btnStartDraw_clicked();
    QTimer::singleShot(500, this, SLOT(on_btnStartDraw_clicked()));
    // gDataManager->DrawHG(ui->boxDrawCh->value());
    gDataManager->Draw(ui->boxDrawCh->value(), (DrawOption)GetDrawOption());
    if (fdrawWin)
        fdrawWin->Update();

    return true;
}

void FEEControlWin::ForceStartDAQ(int nCount, QTime daqTime, int msBufferWaiting, int leastBufferEvent, bool clearBeforeDAQ)
{
    fDAQSettingCount = nCount;
    fDAQSettingTime = daqTime;
    fDAQBufferSleepms = msBufferWaiting;
    fDAQBufferLeastEvents = leastBufferEvent;
    fFlagClearQueue = clearBeforeDAQ;

    ui->boxDAQEvent->setValue(nCount);
    ui->timeDAQSetting->setTime(daqTime);
    ui->boxBufferWait->setValue(fDAQBufferSleepms);
    ui->boxLeastEvents->setValue(fDAQBufferLeastEvents);

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

    ui->boxClearQueue->setChecked(fFlagClearQueue);
    emit startDAQSignal(this);
    fDAQClock.start(1000);
    fDAQIsRunning = 1;

    // GUI Setting
    ui->btnFileInit->setEnabled(false);
    ui->btnFileClose->setEnabled(false);
    ui->btnDAQStop->setEnabled(true);
    ui->grpDAQStart->setEnabled(false);
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

    ui->btnDAQStop->setEnabled(false);
    ui->btnFileClose->setEnabled(true);

    fDAQRuningFlag = 0;
    fDAQIsRunning = 0;

    fDAQClock.stop();
    update_DAQClock();

    CloseSaveFile();
    // on_btnFileClose_clicked();
    on_btnHVOFF_clicked();

    // Tell other class that DAQ is done
    emit stopDAQSignal();
}

void FEEControlWin::StopDAQ()
{
    if (fDAQRuningFlag)
    {
        fDAQRuningFlag = 0;
        gBoard->SetFifoReadBreak();
    }
}

void FEEControlWin::on_btnDAQStop_clicked()
{
    StopDAQ();
    emit forceStopDAQSignal();
}

#include "General.h"
void FEEControlWin::handle_DAQRequest(UserDefine::DAQRequestInfo *daq)
{
    TryStartDAQ(daq->sPath, daq->sFileName, daq->nDAQCount, daq->DAQTime, daq->msBufferSleep, daq->leastBufferEvent, daq->clearQueueFlag);
}

// DAQ control end

// CITIROC Configuration control
void FEEControlWin::SelectLogic(int logic)
{
    fpbtngrpLogic->button(logic)->setChecked(1);
    gBoard->logic_select(logic);

    ui->brsMessage->setTextColor(greenColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("Logic Selected: "));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
    ui->brsMessage->append(tr("logic %1").arg(logic));
}

int FEEControlWin::GetSelectLogic()
{
    return fpbtngrpLogic->checkedId();
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

bool FEEControlWin::Modify_SP_CITIROC_BiasDAC(const std::vector<std::pair<int, int>> &vStatus)
{
    if (!gParser->sConfigValidate())
        return false;
    for (int i = 0; i < vStatus.size(); i++)
    {
        auto p = vStatus[i];
        int ch = p.first, bias = p.second;
        if (ch < 0 || ch > 32)
            return false;
        if (bias < 0 || bias > 255)
            return false;
        gParser->SetBiasDAC(ch, bias);
    }

    SendCITIROCConfig();
    PrintToScreen();
    return true;
}

bool FEEControlWin::Modify_SP_CITIROC_BiasDAC(int ch, int dac)
{
    if (!gParser->sConfigValidate())
        return false;
    if (ch < 0 || ch > 32)
        return false;
    if (dac < 0 || dac > 255)
        return false;
    gParser->SetBiasDAC(ch, dac);
    SendCITIROCConfig();
    PrintToScreen();
    return true;
}

bool FEEControlWin::Modify_SP_CITIROC_HGAmp(const std::vector<std::pair<int, int>> &vStatus)
{
    if (!gParser->sConfigValidate())
        return false;
    for (int i = 0; i < vStatus.size(); i++)
    {
        auto p = vStatus[i];
        int ch = p.first, amp = p.second;
        if (ch < 0 || ch > 32)
            return false;
        if (amp < 0 || amp > 63)
            return false;
        gParser->Set_AMP_HG_DAC(ch, amp);
    }

    SendCITIROCConfig();
    PrintToScreen();
    return true;
}

bool FEEControlWin::Modify_SP_CITIROC_HGAmp(int ch, int dac)
{
    if (!gParser->sConfigValidate())
        return false;
    if (ch < 0 || ch > 32)
        return false;
    if (dac < 0 || dac > 63)
        return false;
    gParser->Set_AMP_HG_DAC(ch, dac);
    SendCITIROCConfig();
    PrintToScreen();
    return true;
}

bool FEEControlWin::Modify_SP_CITIROC_LGAmp(const std::vector<std::pair<int, int>> &vStatus)
{
    if (!gParser->sConfigValidate())
        return false;
    for (int i = 0; i < vStatus.size(); i++)
    {
        auto p = vStatus[i];
        int ch = p.first, amp = p.second;
        if (ch < 0 || ch > 32)
            return false;
        if (amp < 0 || amp > 63)
            return false;
        gParser->Set_AMP_LG_DAC(ch, amp);
    }

    SendCITIROCConfig();
    PrintToScreen();
    return true;
}

bool FEEControlWin::Modify_SP_CITIROC_LGAmp(int ch, int dac)
{
    if (!gParser->sConfigValidate())
        return false;
    if (ch < 0 || ch > 32)
        return false;
    if (dac < 0 || dac > 63)
        return false;
    gParser->Set_AMP_LG_DAC(ch, dac);
    SendCITIROCConfig();
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

        gBoard->GenerateChMask(ch, fcbChannelMask[ch]->isChecked(), fChannelMasks);
    }
    gParser->SetDiscDAC1(ui->boxDiscDAC1->value());
    gParser->SetDiscDAC2(ui->boxDiscDAC2->value());

    return true;
}

bool FEEControlWin::SendCITIROCConfig()
{
    auto rtn = gBoard->SendConfig(gParser);
    std::cout << "Test: send config return " << rtn << std::endl;
    if (rtn)
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
    path += "/../MuonTestControl/Configuration/SaveConfigs/";
    QDir dir;
    if (dir.exists(path))
    {
        dir.mkpath(path);
    }
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

//! Draw Control
void FEEControlWin::SetDrawChannel(int channel)
{
    ui->boxDrawCh->setValue(channel);
}

int FEEControlWin::GetDrawChannel()
{
    return ui->boxDrawCh->value();
}

bool FEEControlWin::SetDrawOption(int option)
{
    if (option < 0 || option > 2)
        return false;
    fpbtngrpDrawOption->button(option)->setChecked(1);
    return true;
}

int FEEControlWin::GetDrawOption()
{
    return fpbtngrpDrawOption->checkedId();
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
    // gReadManager->DrawHG(ui->boxDrawCh->value());
    gReadManager->Draw(ui->boxDrawCh->value(), (DrawOption)GetDrawOption());
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
    fdrawWin->SetOccupied(NULL, false);
}

void FEEControlWin::handle_ContinousDraw()
{
    static int ch = -1;
    static DrawOption option = DrawOption::HGDataDraw;

    if (ch != ui->boxDrawCh->value() || (GetDrawOption() != option && GetDrawOption() != -1))
    {
        ch = ui->boxDrawCh->value();
        option = (DrawOption)GetDrawOption();
        // std::cout << option << std::endl;
        // gDataManager->DrawHG(ui->boxDrawCh->value());
        gDataManager->Draw(ui->boxDrawCh->value(), (DrawOption)GetDrawOption());
    }
    if (!fdrawWin->isHidden())
        fdrawWin->Update();
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

void FEEControlWin::handle_DrawOption_Changed()
{
    if (fDrawFlag && fdrawWin && !fdrawWin->isHidden())
    {
        fdrawWin->SetDrawChannel(GetDrawChannel());
        fdrawWin->SetDrawOption(GetDrawOption());
    }
}
// Draw Control End

// FEE Mask Control
void FEEControlWin::on_btnMask_clicked()
{
    ScanMaskFromSpinbox();
    gBoard->send_ch_masks(fChannelMasks);

    ui->brsMessage->setTextColor(QColor(0, 255, 0));
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append("Sent Masks: " + QString::number(fChannelMasks));
}

void FEEControlWin::PrintMaskToScreen()
{
    DisconnectMasks();
    uint8_t gr0 = (uint8_t)fChannelMasks;
    uint8_t gr1 = (uint8_t)(fChannelMasks >> 8);
    uint8_t gr2 = (uint8_t)(fChannelMasks >> 16);
    uint8_t gr3 = (uint8_t)(fChannelMasks >> 24);

    ui->lblMask0->setValue(gr0);
    ui->lblMask1->setValue(gr1);
    ui->lblMask2->setValue(gr2);
    ui->lblMask3->setValue(gr3);

    for (int ch = 0; ch < 32; ch++)
    {
        fcbChannelMask[ch]->setChecked(FEEControl::GetMask(ch, fChannelMasks));
    }
    ConnectMasks();
}

void FEEControlWin::ScanMaskFromCheckbox()
{
    for (int ch = 0; ch < 32; ch++)
    {
        FEEControl::GenerateChMask(ch, fcbChannelMask[ch]->isChecked(), fChannelMasks);
    }
}

void FEEControlWin::ScanMaskFromSpinbox()
{
    uint32_t gr0 = ((uint8_t)ui->lblMask0->value());
    uint32_t gr1 = ((uint8_t)ui->lblMask1->value()) << 8;
    uint32_t gr2 = ((uint8_t)ui->lblMask2->value()) << 16;
    uint32_t gr3 = ((uint8_t)ui->lblMask3->value()) << 24;

    fChannelMasks = gr0 + gr1 + gr2 + gr3;
}

void FEEControlWin::handle_ScanSpinboxMask()
{
    ScanMaskFromSpinbox();
    PrintMaskToScreen();
}

void FEEControlWin::handle_ScanCheckboxMask()
{
    ScanMaskFromCheckbox();
    PrintMaskToScreen();
}

void FEEControlWin::ConnectMasks()
{
    connect(ui->lblMask0, &QSpinBox::textChanged, this, &FEEControlWin::handle_ScanSpinboxMask);
    connect(ui->lblMask1, &QSpinBox::textChanged, this, &FEEControlWin::handle_ScanSpinboxMask);
    connect(ui->lblMask2, &QSpinBox::textChanged, this, &FEEControlWin::handle_ScanSpinboxMask);
    connect(ui->lblMask3, &QSpinBox::textChanged, this, &FEEControlWin::handle_ScanSpinboxMask);

    for (int ch = 0; ch < 32; ch++)
    {
        connect(fcbChannelMask[ch], &QCheckBox::toggled, this, &FEEControlWin::handle_ScanCheckboxMask);
    }
}

void FEEControlWin::DisconnectMasks()
{
    disconnect(ui->lblMask0, &QSpinBox::textChanged, this, &FEEControlWin::handle_ScanSpinboxMask);
    disconnect(ui->lblMask1, &QSpinBox::textChanged, this, &FEEControlWin::handle_ScanSpinboxMask);
    disconnect(ui->lblMask2, &QSpinBox::textChanged, this, &FEEControlWin::handle_ScanSpinboxMask);
    disconnect(ui->lblMask3, &QSpinBox::textChanged, this, &FEEControlWin::handle_ScanSpinboxMask);

    for (int ch = 0; ch < 32; ch++)
    {
        disconnect(fcbChannelMask[ch], &QCheckBox::toggled, this, &FEEControlWin::handle_ScanCheckboxMask);
    }
}

void FEEControlWin::on_btnClearMask_clicked()
{
    fChannelMasks = 0;
    PrintMaskToScreen();
}

void FEEControlWin::on_btnAllSetMask_clicked()
{
    fChannelMasks = 0xffffffff;
    PrintMaskToScreen();
}

void FEEControlWin::on_btnSignalProbe_clicked()
{
    gBoard->send_ch_probe((char)ui->boxProbeCh->value());
}

// FEE Mask Control END

void FEEControlWin::on_btnClearDraw_clicked()
{
    gDataManager->ClearDraw();
}

#ifdef USE_VISA_CONTROL
#include "VisaDAQControl.h"
#endif
void FEEControlWin::on_btnVISAControl_clicked()
{
#ifdef USE_VISA_CONTROL
    gVisaDAQWin->show();
#endif
}

#ifdef USE_ZABER_MOTION
#include "ZaberControlWidget.h"
#endif

void FEEControlWin::on_btnZaberControl_clicked()
{
#ifdef USE_ZABER_MOTION
    gZaberWindow->show();
#endif
}

//
void FEEControlWin::on_btnGenerateList_clicked()
{
    GenerateGainList();
    GenerateBiasList();
    GenerateChList();

    GenerateLoopOrder();
}

void FEEControlWin::on_btnTest_clicked()
{
    int t1, t2, t3;
    ParseDAQCount(ui->boxTest->value(), t1, t2, t3);
}

void FEEControlWin::on_btnStartLoop_clicked()
{
    if (IsDAQRunning())
        return;
    auto timeLoop = ui->timeLoop->time();
    auto timeDAQ = ui->timeDAQInLoop->time();
    // if loop time is less than daq time+10s, set loop time as daq time + 10s
    double secondAdd = 30;
    // double secondAdd = 0;
    if (timeDAQ.msecsSinceStartOfDay() < 10 * 1000)
    {
        timeDAQ = QTime::fromMSecsSinceStartOfDay(10 * 1000);
        ui->timeDAQInLoop->setTime(timeDAQ);
    }

    if (timeLoop.msecsSinceStartOfDay() - timeDAQ.msecsSinceStartOfDay() < secondAdd * 1000)
    {
        timeLoop = QTime::fromMSecsSinceStartOfDay(timeDAQ.msecsSinceStartOfDay() + secondAdd * 1000);
        ui->timeLoop->setTime(timeLoop);
    }
    ui->timeDAQSetting->setTime(timeDAQ);

    // Start DAQ when the button is pushed.
    StartDAQInLoop();
    fDAQControlTimer.start(timeLoop.msecsSinceStartOfDay());
    fDAQCountDownTimer.start(1000);

    ui->btnStartLoop->setEnabled(0);
    ui->btnStopLoop->setEnabled(1);
    ui->btnClearCounter->setEnabled(0);
    ui->btnGenerateList->setEnabled(0);
}

void FEEControlWin::on_btnStopLoop_clicked()
{
    ui->btnStartLoop->setEnabled(1);
    ui->btnStopLoop->setEnabled(0);
    ui->btnClearCounter->setEnabled(1);
    ui->btnGenerateList->setEnabled(1);

    fDAQControlTimer.stop();
    fDAQCountDownTimer.stop();
}

void FEEControlWin::handle_LoopTimer()
{
    StartDAQInLoop();
}

void FEEControlWin::handle_CountDownTimer()
{
    UpdateCountDown();
}

void FEEControlWin::on_btnClearCounter_clicked()
{
    fLoopCounter = 0;
    ui->lcdLoopCounter->display(fLoopCounter);
    int idxGain, idxBias, idxCh;
    ParseDAQCount(fLoopCounter, idxGain, idxBias, idxCh);
}
