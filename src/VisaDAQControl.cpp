#include "VisaDAQControl.h"
#include "visaapi.h"

#include "ui_VisaDAQControl.h"

// C++
#include <iostream>
#include <string>
#include <sstream>

namespace UserDefine
{
    std::stringstream gss;

    bool ParseLineForVector(std::string sInput, std::vector<double> &vOutput)
    {
        gss.clear();
        gss.str(sInput);
        if (!gss.good())
        {
            std::cout << "Error while parsing: stringstream is bad. " << std::endl;
            return false;
        }
        double j_start = 0, i_step = 0, k_end = 0;
        char c;
        gss >> c;
        if (c != '[')
        {
            std::cout << "Error while parsing: char format error: No left bracket \'[\'" << std::endl;
            return false;
        }
        gss >> j_start;
        if (!gss.good())
        {
            std::cout << "Error while parsing: stringstream is bad. " << std::endl;
            return false;
        }
        gss >> c;
        if (c != ':')
        {
            std::cout << "Error while parsing: char format error: " << c << " Should be \':\'" << std::endl;
            return false;
        }
        gss >> i_step;
        if (!gss.good())
        {
            std::cout << "Error while parsing: stringstream is bad. " << std::endl;
            return false;
        }
        gss >> c;
        if (c != ':')
        {
            std::cout << "Error while parsing: char format error: " << c << " Should be \':\'" << std::endl;
            return false;
        }
        gss >> k_end;
        if (!gss.good())
        {
            std::cout << "Error while parsing: stringstream is bad. " << std::endl;
            return false;
        }
        gss >> c;
        if (c != ']')
        {
            std::cout << "Error while parsing: char format error: No right bracket \']\' " << std::endl;
            return false;
        }

        // Judge whether this three values are valid. If i_step == 0, or k_end-j_start have different signs, return false
        if ((k_end - j_start) * (i_step) < 0)
            return false;
        // If step is 0, return false;
        if (i_step == 0)
            return false;
        vOutput.clear();
        for (double iter = j_start; (iter - j_start) * (iter - k_end) <= 0; iter += i_step)
        {
            vOutput.push_back(iter);
        }
        gss.clear();

        if (vOutput.size() == 0)
            return false;
        return true;
    }

    bool ParseLineForArray(std::string sInput, std::vector<double> &vOutput)
    {
        gss.clear();
        gss.str(sInput);
        double testPoint = 0;
        char c;
        vOutput.clear();
        for (;;)
        {
            gss >> testPoint;
            if (!bool(gss))
                break;
            vOutput.push_back(testPoint);
            if (gss.eof())
                break;
            gss >> c;
            if (c != ',' && c != ':')
            {
                std::cout << "Warning: should use \',\' as delimiter. You are using: \'" << c << "\'. May cause error" << std::endl;
            }
            else if (c == ':')
            {
                std::cout << "Error: Detected \':\', may reveal wrong vector format, pleas check." << std::endl;
                gss.clear();
                vOutput.clear();
                return false;
            }
        }
        gss.clear();
        if (vOutput.size() == 0)
            return false;
        return true;
    }

    bool ParseLine(std::string sInput, std::vector<double> &vOutput)
    {
        gss.clear();
        gss.str(sInput);
        char cFirst;
        gss >> cFirst;
        if (cFirst == '[')
            return ParseLineForVector(sInput, vOutput);
        else if ((cFirst >= '0' && cFirst <= '9') || cFirst == '-')
            return ParseLineForArray(sInput, vOutput);
        return false;
    }
}

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

bool VisaDAQControlWin::GenerateChList()
{
    ui->listCh->clear();
    std::vector<double> temp;
    bool rtn = UserDefine::ParseLine(ui->lineSelectChannel->text().toStdString(), temp);
    if (!rtn)
        return rtn;

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

VisaDAQControl::VisaDAQControl()
{
}

VisaDAQControl::~VisaDAQControl()
{
}

void VisaDAQControlWin::on_btnGenerateList_clicked()
{
    GenerateGainList();
    GenerateAmpList();
    GenerateChList();
}

void VisaDAQControlWin::ClearList()
{
    ui->listAmp->clear();
    ui->listGain->clear();
    ui->listCh->clear();

    fGainList.clear();
    fAmpList.clear();
    fChList.clear();
}

void VisaDAQControlWin::on_btnClearList_clicked()
{
    ClearList();
}

void VisaDAQControlWin::on_boxHGLG_currentIndexChanged(int index)
{
    fHGLGflag = index;
}

const DAQRequestInfo &VisaDAQControlWin::GenerateDAQRequestInfo(DAQRequestInfo &daq)
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

bool VisaDAQControlWin::ParseHandle(int deviceHandle, double &amp, double &gain, int &gainType, DAQRequestInfo &daq)
{
    int nAmp = fAmpList.size(), nGain = fGainList.size();
    int nTotalTest = nAmp * nGain;
    // First fix Amp, change Gain
    if (deviceHandle >= nTotalTest)
        return false;

    ui->listAmp->setCurrentRow(deviceHandle / nGain);
    ui->listGain->setCurrentRow(deviceHandle % nGain);

    amp = fAmpList[deviceHandle / nGain];
    gain = fGainList[deviceHandle % nGain];
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
    auto status = gVisa->SetHigh(amp);
    if (status != 0)
        return false;

    // if (gainType == 0)
    //     rtn = gFEEControlWin->Modify_SP_CITIROC_HGAmp(CombineChDAC(gVisaDAQWin->GetSelectedChannels(), gain));
    // else if (gainType == 1)
    //     rtn = gFEEControlWin->Modify_SP_CITIROC_LGAmp(CombineChDAC(gVisaDAQWin->GetSelectedChannels(), gain));
    // else
    //     return false;

    if (!rtn)
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

void VisaDAQControlWin::StartDAC_R_Test()
{
    handle = 0;
    connect(&fTimer, &QTimer::timeout, this, &VisaDAQControlWin::handle_DACRTest);
    fTimer.start(ui->boxRTestTime->value() * 1000);

    gVisa->SetWaveForm(AFGWaveform::DC);
    handle_DACRTest();
}

void VisaDAQControlWin::handle_DACRTest()
{
    if (handle < fAmpList.size())
    {
        ui->listAmp->setCurrentRow(handle);
        gVisa->SetOffset(fAmpList[handle++]);
    }
    else
    {
        disconnect(&fTimer, &QTimer::timeout, this, &VisaDAQControlWin::handle_DACRTest);
        fTimer.stop();
    }
}
void VisaDAQControlWin::on_btnStartRTest_clicked()
{
    StartDAC_R_Test();
}

void VisaDAQControlWin::on_btnStopRTest_clicked()
{
    fTimer.stop();
}
