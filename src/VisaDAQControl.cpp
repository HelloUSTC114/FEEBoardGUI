#include "VisaDAQControl.h"
#include "visaapi.h"

#include "ui_VisaDAQControl.h"

// C++
#include <iostream>
#include <string>

VisaDAQControlWin::VisaDAQControlWin(QWidget *parent) : QMainWindow(parent),
                                                        ui(new Ui::VisaDAQControlWin)
{
    ui->setupUi(this);

    GenerateWaveformCombox();
    GenerateUnitCombox();
    ui->boxAFGWaveform->setCurrentIndex((int)fWaveform);
    ui->boxAFGFreqUnit->setCurrentIndex((int)fFreqUnit);
    ui->lineAFGFreq->setText(QString::number(fFreq));
}

VisaDAQControlWin::~VisaDAQControlWin()
{
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

VisaDAQControl::VisaDAQControl()
{
    fAFGApi = new VisaAPI;
}

VisaDAQControl::~VisaDAQControl()
{
    delete fAFGApi;
}

bool VisaDAQControl::ProcessDeviceHandle(int deviceHandle)
{
    return true;
}