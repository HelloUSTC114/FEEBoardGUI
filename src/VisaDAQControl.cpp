#include "VisaDAQControl.h"
#include "visaapi.h"

#include "ui_VisaDAQControl.h"

// C++
#include <iostream>
#include <string>
#include <sstream>

namespace UserDefine
{
    stringstream gss;
    bool ParseLine(std::string sInput, std::vector<double> &vOutput)
    {
        gss.clear();
        gss.str(sInput);
        char cFirst;
        gss >> cFirst;
        if (cFirst == '[')
            return ParseLineForVector(sInput, vOutput);
        else if ((cFirst >= '0' && cFirst <= '9' ||) cFirst == '-')
            return ParseLineForArray(sInput, vOutput);
    }

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
            if (!gss.good())
                break;
            vOutput.push_back(testPoint);
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

bool VisaDAQControlWin::GenerateAmpList()
{
    bool rtn = UserDefine::ParseLine(ui->lineAFGAmp->text().toStdString(), fAmpList);
    if (!rtn)
        return rtn;
    for (int i = 0; i < fAmpList.size(); i++)
    {
        QString temp = "Gain: " + QString::number(fAmpList[i]) + " mV";
        ui->listAmp->addItem(temp);
    }

    return rtn;
}

void VisaDAQControlWin::GenerateGainList()
{
    bool rtn = UserDefine::ParseLine(ui->lineCITIROCGain->text().toStdString(), fGainList);
    if (!rtn)
    {
        fGainList.clear();
        fGainList.push_back((double)fDefaultGain);
    }

    for (int i = 0; i < fGainList.size(); i++)
    {
        QString temp = "Gain: " + QString::number(fGainList[i]);
        ui->listGain->addItem(temp);
    }
    return;
}

VisaDAQControl::VisaDAQControl()
{
}

VisaDAQControl::~VisaDAQControl()
{
}

bool VisaDAQControl::ProcessDeviceHandle(int deviceHandle)
{
    return true;
}