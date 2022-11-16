#include "VisaDAQControl.h"
#include "visaapi.h"
#include "General.h"

#include "ui_VisaDAQControl.h"

// C++
#include <iostream>
#include <string>

// Qt
#include <QPalette>

VisaDAQControlWin *VisaDAQControlWin::Instance()
{
    static VisaDAQControlWin *instance = new VisaDAQControlWin;
    return instance;
}

VisaDAQControlWin::VisaDAQControlWin(QWidget *parent) : QMainWindow(parent),
                                                        ui(new Ui::VisaDAQControlWin)
{
    ui->setupUi(this);

    GenerateWaveformCombox();
    GenerateUnitCombox();
    ui->boxAFGWaveform->setCurrentIndex((int)fWaveform);
    ui->boxAFGFreqUnit->setCurrentIndex((int)fFreqUnit);
    ui->lineAFGFreq->setText(QString::number(fFreq));

    fDAQControl = new VisaDAQControl;
    // fDAQControl->moveToThread(&fThread1);
    // connect(&fThread1, &QThread::finished, fDAQControl, &QObject::deleteLater);

    // Test tab
    ui->tabTestChoosen->setCurrentIndex(0);

    ui->tabNLTest->setEnabled(false);
    ui->tabVBTest->setEnabled(false);
}

VisaDAQControlWin::~VisaDAQControlWin()
{
    // fThread1.quit();
    // fThread1.wait();

    delete ui;
}

void VisaDAQControlWin::GenerateWaveformCombox()
{
    for (int wave = 0;; wave++)
    {
        std::string s = ConvertAFGWaveform((AFGWaveform)wave);
        if (s == "")
            break;

        ui->boxAFGWaveform->addItem(QString::fromStdString(s));
    }
}

void VisaDAQControlWin::GenerateUnitCombox()
{
    for (int unit = 0;; unit++)
    {
        std::string s = ConvertAFGFreqUnit((AFGFreqUnit)unit);
        if (s == "")
            break;

        ui->boxAFGFreqUnit->addItem(QString::fromStdString(s));
    }
}

bool VisaDAQControlWin::GenerateAmpList()
{
    ui->listAmp->clear();
    std::vector<double> temp;
    bool rtn = UserDefine::ParseLine(ui->lineAFGAmp->text().toStdString(), temp);
    if (!rtn)
        return rtn;

    fAmpList.clear();
    for (int i = 0; i < temp.size(); i++)
    {
        if (temp[i] >= 50 && temp[i] < 5000)
        {
            fAmpList.push_back(temp[i]);
        }
    }
    for (int i = 0; i < fAmpList.size(); i++)
    {
        QString temp = "AFG: " + QString::number(fAmpList[i]) + " mV";
        ui->listAmp->addItem(temp);
    }

    return rtn;
}

bool VisaDAQControlWin::GenerateChListVB()
{
    ui->listChVB->clear();
    std::vector<double> temp;
    bool rtn = UserDefine::ParseLine(ui->lineSelectChannelVB->text().toStdString(), temp);
    if (!rtn)
        return rtn;

    fChListVB.clear();
    for (int i = 0; i < temp.size(); i++)
    {
        if (temp[i] >= 0 && temp[i] < 32)
        {
            fChListVB.push_back(temp[i]);
        }
    }

    for (int i = 0; i < fChListVB.size(); i++)
    {
        QString temp = "Channel: " + QString::number(fChListVB[i]);
        ui->listChVB->addItem(temp);
    }

    return rtn;
}

bool VisaDAQControlWin::GenerateChListNL()
{
    ui->listChNL->clear();
    std::vector<double> temp;
    bool rtn = UserDefine::ParseLine(ui->lineSelectChannelNL->text().toStdString(), temp);
    if (!rtn)
        return rtn;

    fChListNL.clear();
    for (int i = 0; i < temp.size(); i++)
    {
        if (temp[i] >= 0 && temp[i] < 32)
        {
            fChListNL.push_back(temp[i]);
        }
    }

    for (int i = 0; i < fChListNL.size(); i++)
    {
        QString temp = "Channel: " + QString::number(fChListNL[i]);
        ui->listChNL->addItem(temp);
    }

    return rtn;
}

void VisaDAQControlWin::GenerateGainList()
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
        QString temp = QString::fromStdString(GetAmpType()) + " " + QString::number(fGainList[i]);
        ui->listGain->addItem(temp);
    }
    return;
}

void VisaDAQControlWin::GenerateBiasList()
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
        QString temp = QString::fromStdString("Bias ") + QString::number(fBiasList[i]);
        ui->listBias->addItem(temp);
    }
    return;
}

std::string VisaDAQControlWin::GetAmpType()
{
    return ui->boxHGLG->currentText().toStdString();
}

bool VisaDAQControlWin::SetAmpType(int type)
{
    if (type < 0 || type > 1)
        return false;
    fHGLGflag = type;
    ui->boxHGLG->setCurrentIndex(type);
    return true;
}

void VisaDAQControlWin::on_btnGenerateListVB_clicked()
{
    // GenerateGainList();
    GenerateBiasList();
    // GenerateAmpList();
    GenerateChListVB();
}

void VisaDAQControlWin::ClearListVB()
{
    // ui->listAmp->clear();
    // ui->listGain->clear();
    ui->listBias->clear();
    ui->listChVB->clear();

    // fGainList.clear();
    fBiasList.clear();
    // fAmpList.clear();
    fChListVB.clear();
}

void VisaDAQControlWin::on_btnGenerateListNL_clicked()
{
    GenerateGainList();
    // GenerateBiasList();
    GenerateAmpList();
    GenerateChListNL();
}

void VisaDAQControlWin::ClearListNL()
{
    ui->listAmp->clear();
    ui->listGain->clear();
    // ui->listBias->clear();
    ui->listChNL->clear();

    fGainList.clear();
    // fBiasList.clear();
    fAmpList.clear();
    fChListNL.clear();
}

void VisaDAQControlWin::on_btnClearListVB_clicked()
{
    ClearListVB();
}

void VisaDAQControlWin::on_btnClearListNL_clicked()
{
    ClearListNL();
}

void VisaDAQControlWin::on_boxHGLG_currentIndexChanged(int index)
{
    fHGLGflag = index;
}

// NL Test & Device control
const UserDefine::DAQRequestInfo &VisaDAQControlWin::GenerateDAQRequestInfo(UserDefine::DAQRequestInfo &daq)
{
    daq.DAQTime = ui->timeDAQSetting->time();
    daq.leastBufferEvent = ui->boxLeastEvents->value();
    daq.msBufferSleep = ui->boxBufferWait->value();
    daq.nDAQCount = ui->boxDAQEvent->value();
    daq.sFileName = ui->lblFileName->text().toStdString();
    daq.sPath = fsFilePath.toStdString();
    daq.clearQueueFlag = ui->boxClearQueue->isChecked();
    return daq;
}

#include <QDir>
#include <QFileDialog>
void VisaDAQControlWin::on_btnPath_clicked()
{
    fsFilePath = QFileDialog::getExistingDirectory(this, tr("Choosing File Path"), QDir::currentPath() + "/../MuonTestControl/Data");
    if (fsFilePath == "")
        return;
}

bool VisaDAQControlWin::ParseHandle(int deviceHandle, double &amp, double &gain, int &gainType, UserDefine::DAQRequestInfo &daq)
{
    int nAmp = fAmpList.size(), nGain = fGainList.size();
    int nTotalTest = nAmp * nGain;
    // First fix Gain, change amp
    if (deviceHandle >= nTotalTest)
        return false;

    std::cout << deviceHandle / nAmp << '\t' << deviceHandle % nAmp << std::endl;
    ui->listGain->setCurrentRow(deviceHandle / nAmp);
    ui->listAmp->setCurrentRow(deviceHandle % nAmp);

    amp = fAmpList[deviceHandle % nAmp];
    gain = fGainList[deviceHandle / nAmp];
    gainType = fHGLGflag;
    GenerateDAQRequestInfo(daq);

    std::string sHGLG;
    if (fHGLGflag == 0)
        sHGLG = "HG";
    else
        sHGLG = "LG";

    char buf[256];
    sprintf(buf, "-Amp-%.2f-%s-%d", amp, sHGLG.c_str(), (int)gain);
    daq.sFileName += (std::string)buf;
    return true;
}

bool VisaDAQControlWin::JudgeLastLoop(int deviceHandle)
{
    int nAmp = fAmpList.size(), nGain = fGainList.size();
    int nTotalTest = nAmp * nGain;
    if (deviceHandle == nTotalTest - 1)
        return true;
    else
        return false;
}

VisaDAQControl::VisaDAQControl()
{
}

VisaDAQControl::~VisaDAQControl()
{
}

#include "FEEControlWidget.h"
std::vector<std::pair<int, int>> CombineChDAC(std::vector<int> vCh, int dac)
{
    std::vector<std::pair<int, int>> vReturn;
    for (int i = 0; i < vCh.size(); i++)
    {
        vReturn.push_back(std::pair<int, int>(vCh[i], dac));
    }
    return vReturn;
}

bool VisaDAQControl::ProcessDeviceHandle(int deviceHandle)
{
    double amp, gain;
    int gainType;
    bool rtn = gVisaDAQWin->ParseHandle(deviceHandle, amp, gain, gainType, fDAQInfo);
    if (!rtn)
        return false;
    auto status = gAFGVisa->SetHigh(amp);
    if (deviceHandle == 0)
    {
        status = gAFGVisa->SetChannelStatus(1, 1);
        // gAFGVisa->SetWaveForm(AFGWaveform::USER1);
    }
    if (status != 0)
        return false;

    auto rtn1 = gFEEControlWin->Modify_SP_CITIROC_HGAmp(CombineChDAC(gVisaDAQWin->GetSelectedChannelsNL(), gain));
    auto rtn2 = gFEEControlWin->Modify_SP_CITIROC_LGAmp(CombineChDAC(gVisaDAQWin->GetSelectedChannelsNL(), gain));

    _sleep(500);
    // if (gainType == 0)
    // else if (gainType == 1)
    // else
    // return false;

    if (!(rtn1 && rtn2))
        return false;

    return true;
}

bool VisaDAQControl::JudgeLastLoop(int deviceHandle)
{
    return gVisaDAQWin->JudgeLastLoop(deviceHandle);
}

void VisaDAQControlWin::on_btnADCNL_clicked()
{
    fDAQControl->StartTest();
}

void VisaDAQControlWin::on_btnStopNL_clicked()
{
    fDAQControl->ForceStopDevice();
}

// DAC Test
void VisaDAQControlWin::StartDAC_R_Test()
{
    handle = 0;
    connect(&fTimer, &QTimer::timeout, this, &VisaDAQControlWin::handle_DACRTest);
    fTimer.start(ui->boxRTestTime->value() * 1000);

    gAFGVisa->SetWaveForm(AFGWaveform::DC);
    gAFGVisa->SetChannelStatus(1, 1);
    handle_DACRTest();
}

void VisaDAQControlWin::handle_DACRTest()
{
    if (handle < fAmpList.size())
    {
        ui->listAmp->setCurrentRow(handle);
        gAFGVisa->SetOffset(fAmpList[handle++]);
    }
    else
    {
        StopDAC_R_Test();
    }
}

void VisaDAQControlWin::StopDAC_R_Test()
{
    disconnect(&fTimer, &QTimer::timeout, this, &VisaDAQControlWin::handle_DACRTest);
    fTimer.stop();
}

#include <QtConcurrent/QtConcurrentRun>
void VisaDAQControlWin::StartDAC_V_Test()
{
    fDACVTestBreakFlag = 0;
    handleDACV = 0;
    ui->listBias->setCurrentRow(handleDACV);

    int chTemp = ui->listChVB->currentRow();
    std::cout << chTemp << std::endl;
    handleDACVch = (chTemp > 31 || chTemp < 0) ? 0 : chTemp;
    ui->listChVB->setCurrentRow(handleDACVch);
    ui->lineChMea->setText(QString::number(fChListVB[handleDACVch]));
    ui->listChVB->setEnabled(0);
    ui->btnStartVTest->setEnabled(0);
    ui->btnNextChReady->setEnabled(0);

    // connect(&fTimer, &QTimer::timeout, this, &VisaDAQControlWin::handle_DACVTest);
    // fTimer.start(ui->boxVSamplePoints->value() * 300);
    handle_DACVTest();
}

void VisaDAQControlWin::StopDAC_V_Test()
{
    fDACVTestBreakFlag = 0;

    // disconnect(&fTimer, &QTimer::timeout, this, &VisaDAQControlWin::handle_DACVTest);
    ui->listChVB->setEnabled(1);
    ui->btnStartVTest->setEnabled(1);
    ui->btnNextChReady->setEnabled(1);
    fTimer.stop();
}

double ProcessVoltageTest(int channel, int biasDAC, int nSamples, std::string sFolder);

#include <QFuture>
void VisaDAQControlWin::handle_DACVTest()
{
    static QFuture<double> DACVfuture;

    // fTimer.singleShot(ui->boxVSamplePoints->value() * 300, this, &VisaDAQControlWin::handle_DACVTest);
    if (handleDACV < fBiasList.size())
    {
        // std::cout << "John Test: Start DAC V: " << std::endl;
        // std::cout << "Processing handle: " << handleDACV << std::endl;
        std::string sFolder = "../MuonTestControl/Data/DACVTest/";

        // Wait for previous measuring
        if (handleDACV != 0)
        {
            auto result = DACVfuture.result();
            std::cout << "John Test: totally got " << result << " points" << std::endl;
        }
        ui->listBias->setCurrentRow(handleDACV);
        DACVfuture = QtConcurrent::run(ProcessVoltageTest, fChListVB[handleDACVch], fBiasList[handleDACV++], ui->boxVSamplePoints->value(), sFolder);

        if (!fDACVTestBreakFlag)
            // std::cout << "Test" << std::endl;
            fTimer.singleShot(ui->boxVSamplePoints->value() * 50, this, &VisaDAQControlWin::handle_DACVTest);
        else
            StopDAC_V_Test();
    }
    else
    {
        StopDAC_V_Test();
    }
}

void VisaDAQControlWin::on_btnStartRTest_clicked()
{
    StartDAC_R_Test();
}

void VisaDAQControlWin::on_btnStopRTest_clicked()
{
    StopDAC_R_Test();
}

void VisaDAQControlWin::on_btnStartVTest_clicked()
{
    StartDAC_V_Test();
}

void VisaDAQControlWin::on_btnStopVTest_clicked()
{
    fDACVTestBreakFlag = 1;
    // StopDAC_V_Test();
}

#include <QMutex>
#include <fstream>
double VoltageTest(int nSamplePoints, std::string fileName, std::string sHeader);

double ProcessVoltageTest(int channel, int biasDAC, int nSamples, std::string sFolder)
{
    QMutex mutex;
    mutex.lock();
    gFEEControlWin->Modify_SP_CITIROC_BiasDAC(channel, biasDAC);
    _sleep(1000);

    std::string fileName = sFolder + "Channel-" + std::to_string(channel) + ".txt";

    // Header is the first word in the line, input bias info
    std::string sHeader = std::to_string(biasDAC);
    auto rtn = VoltageTest(nSamples, fileName, sHeader);
    mutex.unlock();
    return rtn;
}

#include <time.h>
double VoltageTest(int nSamplePoints, std::string fileName, std::string sHeader)
{
    std::ofstream fout(fileName, std::ios::app);
    if (!fout.is_open())
    {
        std::cout << "Warning: cannot open file: " << fileName << std::endl;
        return 0;
    }

    // Start Sampling
    std::vector<double> vResult;
    gAgi1344Visa->InitMeasure34410();
    gAgi1344Visa->Measure34410(vResult);

    // Output
    fout << sHeader << '\t';
    fout << vResult.size() << '\t';
    for (int i = 0; i < vResult.size(); i++)
    {
        fout << vResult[i] << '\t';
    }
    fout << std::endl;
    fout.close();
    return vResult.size();
}

double VoltageTest_Previous(int nSamplePoints, std::string fileName, std::string sHeader)
{
    std::ofstream fout(fileName, std::ios::app);
    if (!fout.is_open())
    {
        std::cout << "Warning: cannot open file: " << fileName << std::endl;
        return 0;
    }

    // Start Sampling
    std::vector<double> vResult;
    gAgi1344Visa->InitMeasure();
    auto start = clock();
    for (int i = 0; i < nSamplePoints; i++)
    {
        auto start0 = clock();
        double mea = gAgi1344Visa->MeasureOnce();
        std::cout << i << '\t' << mea << '\t' << "Start time: " << start0 << "\t running time: " << clock() - start0 << std::endl;
        if (mea != 0)
            vResult.push_back(mea);
    }
    std::cout << "Total running time: " << clock() - start << '\t' << std::endl;

    // Output
    fout << sHeader << '\t';
    fout << vResult.size() << '\t';
    for (int i = 0; i < vResult.size(); i++)
    {
        fout << vResult[i] << '\t';
    }
    fout << std::endl;
    fout.close();
    return vResult.size();
}

void VisaDAQControlWin::on_btnNextChReady_clicked()
{
    handleDACVch++;
    if (handleDACVch >= fChListVB.size())
    {
        handleDACVch = 0;
    }
    ui->lineChMea->setText(QString::number(fChListVB[handleDACVch]));
    ui->listChVB->setCurrentRow(handleDACVch);
    if (!handleDACVch)
        return;
    StartDAC_V_Test();
}

void VisaDAQControlWin::on_btnAFGConnect_clicked()
{
    std::string sDeviceName = "TCPIP::" + ui->lineAFGIP->text().toStdString() + "::INSTR";
    gAFGVisa->TryConnectDevice(sDeviceName);
    if (gAFGVisa->IsConnected())
    {
        ui->lblAFGLED->setStyleSheet("background-color:rgb(0,255,0)");
        fAFGReady = 1;
    }
    else
    {
        ui->lblAFGLED->setStyleSheet("background-color:rgb(255,0,0)");
        fAFGReady = 0;
    }
}

void VisaDAQControlWin::on_btnAgiConnect_clicked()
{
    std::string sDeviceName = "TCPIP::" + ui->lineAgiIP->text().toStdString() + "::INSTR";
    gAgi1344Visa->TryConnectDevice(sDeviceName);
    if (gAgi1344Visa->IsConnected())
    {
        ui->lblAgiLED->setStyleSheet("background-color:rgb(0,255,0)");
        fAgiReady = 1;
    }
    else
    {
        ui->lblAgiLED->setStyleSheet("background-color:rgb(255,0,0)");
        fAgiReady = 0;
    }
}

void VisaDAQControlWin::on_btnFEEConnect_clicked()
{
    if (gFEEControlWin->IsConnected())
    {
        ui->lblFEELED->setStyleSheet("background-color:rgb(0,255,0)");
        fFEEReady = 1;
    }
    else
    {
        ui->lblFEELED->setStyleSheet("background-color:rgb(255,0,0)");
        fFEEReady = 0;
    }
}

void VisaDAQControlWin::on_btnDeviceCheck_clicked()
{
    if (!fFEEReady)
        on_btnFEEConnect_clicked();
    if (!fAFGReady)
        on_btnAFGConnect_clicked();
    if (!fAgiReady)
        on_btnAgiConnect_clicked();
    if (!fFEEReady)
        return;
    if (fAFGReady)
        ui->tabNLTest->setEnabled(true);
    if (fAgiReady)
        ui->tabVBTest->setEnabled(true);
}

void VisaDAQControlWin::on_btnDACVPath_clicked()
{
}
