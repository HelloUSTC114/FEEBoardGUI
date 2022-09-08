#ifndef VISADAQCONTROL_H
#define VISADAQCONTROL_H

#include <QMainWindow>
#include "VDeviceController.h"
#include "visaapi.h"

// C++
#include <string>
#include <vector>

namespace Ui
{
    class VisaDAQControlWin;
}

namespace UserDefine
{
    bool ParseLine(std::string sInput, std::vector<double> &vOutput);
}

class VisaDAQControlWin : public QMainWindow
{
    Q_OBJECT
    friend class VisaDAQControl;

public:
    explicit VisaDAQControlWin(QWidget *parent = nullptr);
    ~VisaDAQControlWin();

    void LockGUI();
    void UnLockGUI();

    void SetWaveform(AFGWaveform wave);
    void SetFreqUnit(AFGFreqUnit unit);

private:
    Ui::VisaDAQControlWin *ui;
    void GenerateWaveformCombox();
    void GenerateUnitCombox();
    void GenerateGainList();
    bool GenerateAmpList();

    std::vector<double> fGainList;
    std::vector<double> fAmpList;
    const int fDefaultGain = 30;

    AFGFreqUnit fFreqUnit = AFGFreqUnit::kHz;
    AFGWaveform fWaveform = AFGWaveform::USER1;
    double fFreq = 1;
};

class VisaDAQControl : public VDeviceController
{
public:
    VisaDAQControl();
    ~VisaDAQControl();

    // Functions for continuous DAQ and device control
    bool TryStart();
    bool ProcessStop();
    bool ParseHandle(int deviceHandle);
    bool ProcessDeviceHandle(int deviceHandle) override;

    // functions for only device control

private:
    VisaAPI *fAFGApi;
};

#endif // VISADAQCONTROL_H
