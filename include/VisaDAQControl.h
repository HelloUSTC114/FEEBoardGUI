#ifndef VISADAQCONTROL_H
#define VISADAQCONTROL_H

#include <QMainWindow>
#include "VDeviceController.h"
#include "visaapi.h"

namespace Ui
{
    class VisaDAQControlWin;
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

    AFGFreqUnit fFreqUnit = AFGFreqUnit::kHz;
    AFGWaveform fWaveform = AFGWaveform::USER1;
    double fFreq = 1;
};

class VisaDAQControl : public VDeviceController
{
public:
    VisaDAQControl();
    ~VisaDAQControl();
    bool ParseHandle(int deviceHandle);

    bool ProcessStart();
    bool ProcessStop();

    bool ProcessDeviceHandle(int deviceHandle) override;

private:
    VisaAPI* fAFGApi;
};

#endif // VISADAQCONTROL_H
