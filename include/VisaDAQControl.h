#ifndef VISADAQCONTROL_H
#define VISADAQCONTROL_H

#include "VDeviceController.h"
#include "visaapi.h"

// C++
#include <string>
#include <vector>

// Qt
#include <QString>
#include <QMainWindow>
#include <QThread>
#include <QTimer>

namespace Ui
{
    class VisaDAQControlWin;
}

class VisaDAQControl : public VDeviceController
{
public:
    VisaDAQControl();
    ~VisaDAQControl();

    // Functions for continuous DAQ and device control
    bool TryStart();
    bool ProcessStop();
    bool ProcessDeviceHandle(int deviceHandle) override;
    bool JudgeLastLoop(int deviceHandle) override;

    // functions for only device control

private:
};

#define gVisaDAQWin (VisaDAQControlWin::Instance())
class VisaDAQControlWin : public QMainWindow
{
    Q_OBJECT
    friend class VisaDAQControl;

public:
    ~VisaDAQControlWin();
    static VisaDAQControlWin *Instance();

    void LockGUI();
    void UnLockGUI();

    // DAQ
    QString GetPath() { return fsFilePath; }
    const UserDefine::DAQRequestInfo &GenerateDAQRequestInfo(UserDefine::DAQRequestInfo &daq);

    // AFG3000
    void SetWaveform(AFGWaveform wave);
    void SetFreqUnit(AFGFreqUnit unit);

    // FEE Board
    std::string GetAmpType();
    bool SetAmpType(int type);
    const std::vector<int> &GetSelectedChannels() { return fChList; }

    // DAQ & FEE Control Parser
    bool ParseHandle(int deviceHandle, double &amp, double &gain, int &gainType, UserDefine::DAQRequestInfo &daq);
    bool JudgeLastLoop(int deviceHandle);

    // DAC R test
    void StartDAC_R_Test();
    void StopDAC_R_Test();

    // DAC V test
    void StartDAC_V_Test();
    void StopDAC_V_Test();

private slots:

    void on_btnGenerateList_clicked();
    void on_btnClearList_clicked();

    void on_boxHGLG_currentIndexChanged(int index);

    void on_btnPath_clicked();

    void on_btnADCNL_clicked();

    void on_btnStopNL_clicked();

    void handle_DACRTest();

    void handle_DACVTest();

    void on_btnStartRTest_clicked();

    void on_btnStopRTest_clicked();

    void on_btnStartVTest_clicked();

    void on_btnStopVTest_clicked();

    void on_btnNextChReady_clicked();

private:
    explicit VisaDAQControlWin(QWidget *parent = nullptr);

    Ui::VisaDAQControlWin *ui;

    // DAQ Setting
    QString fsFileName = "Data";
    QString fsFilePath = "../MuonTestControl/Data";

    // AFG3000
    void GenerateWaveformCombox();
    void GenerateUnitCombox();
    AFGFreqUnit fFreqUnit = AFGFreqUnit::kHz;
    AFGWaveform fWaveform = AFGWaveform::USER1;
    double fFreq = 1;

    // FEE
    void GenerateGainList();
    void GenerateBiasList();
    bool GenerateAmpList();
    bool GenerateChList();
    void ClearList();
    std::vector<int> fGainList;
    std::vector<int> fBiasList;
    std::vector<double> fAmpList;
    std::vector<int> fChList;
    const int fDefaultGain = 30;
    const int fDefaultBias = 100;
    int fHGLGflag = 0;

    // Qt Thread
    VisaDAQControl *fDAQControl = NULL;

    // DAC R Test
    QTimer fTimer;
    int handle = 0;

    // DAC V Test
    int handleDACV = 0;
    int handleDACVch = 0;
    volatile bool fDACVTestBreakFlag = 0;
};

#endif // VISADAQCONTROL_H
