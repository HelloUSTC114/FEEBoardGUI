#include "FEEControlWidget.h"
#include "./ui_FEEControlWidget.h"

// User
#include "feecontrol.h"
#include "configfileparser.h"

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

FEEControlWin::FEEControlWin(QWidget *parent)
    : QWidget(parent), ui(new Ui::FEEControlWin)
{
    ui->setupUi(this);

    // FEE control Tab
    ui->lineIP->setEnabled(false);
    ui->boxPort->setEnabled(false);

    ui->grpFEEInfo->setEnabled(false);
    ui->grpHVctrl->setEnabled(false);
    ui->grpTMea->setEnabled(false);
    ui->grpTMoni->setEnabled(false);

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
    ui->grpFEEInfo->setEnabled(true);
    ui->grpHVctrl->setEnabled(true);
    ui->grpTMea->setEnabled(true);
    ui->grpTMoni->setEnabled(true);

    // other tabs
    ui->tabCITIROC->setEnabled(true);

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

    gBoard->InitPort(ip, port);
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
// FEE Control END

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

        gBoard->GenerateChMask(ch, fcbChannelMask[ch]->isChecked(), fChannelMasks);
    }
    gParser->SetDiscDAC1(ui->boxDiscDAC1->value());
    gParser->SetDiscDAC2(ui->boxDiscDAC2->value());

    return true;
}

bool FEEControlWin::SendCITIROCConfig()
{
    auto rtn = gBoard->SendConfig(gParser);
    if (!rtn)
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

void FEEControlWin::on_btnMask_clicked()
{
    ScanFromScreen();
    gBoard->set_channel_mask(fChannelMasks);

    ui->brsMessage->setTextColor(QColor(0, 255, 0));
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append("Sent Masks: " + QString::number(fChannelMasks));
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
    gBoard->BoardExit();

    ui->brsMessage->setTextColor(greenColor);
    ui->brsMessage->setFontWeight(QFont::Bold);
    ui->brsMessage->append(tr("Board Exited."));

    ui->brsMessage->setTextColor(blackColor);
    ui->brsMessage->setFontWeight(QFont::Normal);
}
