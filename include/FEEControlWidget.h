#ifndef WIDGET_H
#define WIDGET_H
#pragma warning(disable : 4996)

// Qt
#include <QWidget>
#include <QThread>
#include <QTimer>
#include <QTime>

// C++ STL
#include <string>
#include <vector>

namespace UserDefine
{
    struct DAQRequestInfo;
}

class FEEControlWin;
class PlotWindow;
class ROOTDraw;
class FTAnalyzerWin;

class QCheckBox;
class QLabel;
class QSpinBox;
class QButtonGroup;
class QLabel;
class QLabel;

QT_BEGIN_NAMESPACE
namespace Ui
{
    class FEEControlWin;
}
QT_END_NAMESPACE

// DAQ Starting Processing class
class DAQRuning : public QObject
{
    Q_OBJECT
signals:
    void DAQStopSignal(int nDAQLoop);
    void UpdateDAQCount(int nDAQCount);
public slots:
    void startDAQ(FEEControlWin *w);

private:
    bool JudgeLoopFlag(FEEControlWin *w, int nEventCount);
};

class FEEControlWin : public QWidget
{
    Q_OBJECT
    friend class DAQRuning;

public:
    ~FEEControlWin();
    static FEEControlWin *Instance(); // Forbidden create FEEControlWin by user
    void setParent(QWidget *parent = nullptr) { QWidget::setParent(parent); }

    //! TODO: In the future, this class should adapt to several boards connections.
    bool TryConnect(std::string sIP, int port);
    bool IsConnected() { return fConnected; }

    /// @brief Try to start DAQ Process
    /// @param sPath SaveFile Folder Path
    /// @param sFileName Save File Name
    /// @param nDAQCount DAQ Count
    /// @param DAQTime DAQ time
    /// @param msBufferSleep FEE board buffer reading waiting time (in ms)
    /// @return whether DAQ start Successfully
    bool TryStartDAQ(std::string sPath, std::string sFileName, int nDAQCount = -1, QTime DAQTime = {0, 0, 0}, int msBufferSleep = 200, int leastBufferEvent = 30, bool clearBeforeDAQ = 1);
    void StopDAQ();
    bool IsDAQRunning() { return fDAQIsRunning; }
    QString GetPath() { return fsFilePath; }
    QString GetFileName() { return fsFileName; }
    QString GetFileNameWithStamp() { return fsFileNameTotal; }

    QDateTime GetFileTimeStamp() { return fFileTimeStamp; };

    double GetRealCount() { return fDAQRealCount; };
    double GetRealTime() { return fDAQRealTime; };
    double GetCountRate() { return fDAQRealTime == 0 ? 0 : fDAQRealCount / (double)fDAQRealTime * 1000; };

    /// @brief Set Draw Channel in public method
    /// @param channel selected channel
    void SetDrawChannel(int channel);
    int GetDrawChannel();
    bool SetDrawOption(int option);
    //! TODO: Add Draw option in gui, using QButtonGroup
    int GetDrawOption();
    void Test();

    /// @brief FEE Logic Selection
    /// @param logic logic number
    void SelectLogic(int logic);
    int GetSelectLogic(); // Get selected logic in gui

    /// @brief Read, Send, Print to screen , CITIROC configuration file
    /// @param scFile sc configuration file, with path inside
    /// @param pbFile pb configuration file, with path inside
    /// @return whether Read, send, print successfully
    bool RSP_CITIROC_configFile(std::string scFile, std::string pbFile);
    bool Modify_SP_CITIROC_BiasDAC(const std::vector<std::pair<int, int>> &vStatus);
    bool Modify_SP_CITIROC_BiasDAC(int ch, int dac);
    bool Modify_SP_CITIROC_HGAmp(const std::vector<std::pair<int, int>> &vStatus);
    bool Modify_SP_CITIROC_LGAmp(const std::vector<std::pair<int, int>> &vStatus);

signals:
    void startDAQSignal(FEEControlWin *); // FEE Start DAQ signal
    void stopDAQSignal();                 // DAQ Stop signal, tell other class that DAQ is done
    void forceStopDAQSignal();            // DAQ Force stop signal, tell other class that DAQ is interrupted

private slots:
    void handle_connectionBroken(int boardNo); // Handle Connection broken situation

    void on_DAQStoped(int nDAQLoop);     // FEE DAQ Stop slot
    void handle_ContinousDraw();         // Handle draw slot
    void handle_DAQCount(int nDAQCount); // Handle DAQ Count
    void update_DAQClock();              // Handle DAQ Clock

public slots:
    void handle_DAQRequest(UserDefine::DAQRequestInfo *); // Process DAQ signal

private:
    /// @brief Generator, private for now. Will be public and update to several boards control
    /// @param parent
    FEEControlWin(QWidget *parent = nullptr);

private:
    Ui::FEEControlWin *ui;

    // FEE Board
    volatile bool fConnected = false;  // Whether board is connected
    int fCurrentBoardNo = -1; // Current board number

    void PrintConnection(bool flag); // Print Connection information
    void ProcessConnect();           // Process after successfully connected, send logic, send CITIROC config
    void ProcessDisconnect();        // Process after Board Exit
    void PrintHV();                  // Print HV Information
    void PrintT();                   // Print Temerature information
    void PrintClock();               // Print Si570 clock frequency

    QTimer fOnceTimer; // Timer only used for once action, such as monitor starter

    // Count Rate Monitor
    QTimer fCRClock;
    void RetrieveCountOnce();

    // DAQ control

    /// @brief Force to start DAQ, regardless of file initial, focus only on emit start signal, all start methods should contain this method
    /// @param nCount Count number for DAQ, -1 for forever
    /// @param daqTime Count time for DAQ, 00:00:00.000 for forever
    /// @param msBufferWaiting Parameter for Socket communication, if buffer is not long enough, how long (in ms) should be wait.
    void ForceStartDAQ(int nCount, QTime daqTime, int msBufferWaiting, int leastBufferEvent = 30, bool fClearQueue = 1);

    // DAQ File Manager
    bool GenerateROOTFile(); // Generate root file
    void CloseSaveFile();    // Close ROOT File

    QString fsFilePath = "../MuonTestControl/Data"; // Save ROOT File Path
    QString fsFileName = "Data";                    // Save ROOT File Name without time stamp
    QString fsFileNameTotal;                        // Save ROOT File Total Name, with time stamp
    QDateTime fFileTimeStamp;                       //

    bool fDAQIsRunning = 0;            // record whehter DAQ is running
    int fDAQSettingCount = -1;         // -1 means DAQ forever until stopDAQ clicked
    QTime fDAQSettingTime{0, 0, 0, 0}; // DAQ time setting, 0,0,0,0 means forever until stopDAQ clicked
    int fDAQRealTime = 0;              // monitored real DAQ total time
    int fDAQRealCount = 0;             // Monitored read DAQ count
    volatile bool fDAQRuningFlag = 0;  // DAQ runing flag, used to break daq process
    QDateTime fDAQStartTime;           // Record start DAQ time
    QTimer fDAQClock;                  // a 1s clock, aims at updating DAQ time in GUI
    DAQRuning *fDAQProcessor = NULL;   //
    QThread fWorkThread3;              //
    int fDAQBufferSleepms = 200;       // FEE control while DAQ, Wait time of buffer reading
    int fDAQBufferLeastEvents = 30;    // FEE control while DAQ, Only when buffer length larger than this, Fifo data will be readout
    bool fFlagClearQueue = 0;          // FEE control before DAQ, give signal whether clear Queue before DAQ

    // FEE Logic Selection
    QButtonGroup *fpbtngrpLogic; // Single button group of logical selection

    // FEE CITIROC configuration
    QLabel *flblChs[32];
    QSpinBox *fspinsHGAmp[32];
    QSpinBox *fspinsLGAmp[32];
    QSpinBox *fspinsBias[32];
    QCheckBox *fcbBias[32];
    QCheckBox *fcbDisablePA[32];
    QCheckBox *fcbChannelMask[32];
    QLabel *flblHGAmp, *flblLGAmp, *flblBias, *flblBiasBox, *flblPABox, *flblChMaskBox;

    QString sCITIROC_Config_Path, sCITIROC_sc_file, sCITIROC_pb_file;
    bool ReadCITIROC_configFile();
    bool ScanFromScreen();    // Scan from screen, change to gParser
    bool PrintToScreen();     // print data from gParser to screen
    bool SendCITIROCConfig(); // Send CITIROC config

    // Mask control
    uint32_t fChannelMasks = 0;
    void ScanMaskFromCheckbox();
    void ScanMaskFromSpinbox();
    void PrintMaskToScreen();
    void DisconnectMasks();
    void ConnectMasks();

    // Draw Setting
    std::vector<ROOTDraw *> fWinList; // Created ROOTDraw Windows list,
    void DrawSingle();                //
    volatile bool fDrawFlag = 0;      // DAQ Drawing flag
    QTimer fDrawerTimer;              //
    ROOTDraw *fdrawWin = NULL;        //
    QButtonGroup *fpbtngrpDrawOption; // Draw option button group

private slots:
    // FEE Board Control
    void on_btnConnect_clicked();
    void on_btnHVON_clicked();
    void on_btnHPO_clicked();
    void on_btnHVOFF_clicked();
    void on_btnHVSet_clicked();
    void on_btnRegTest_clicked();
    void on_btnTMon_clicked();

    // DAQ Control
    void on_btnPath_clicked();
    void on_btnFileInit_clicked();
    void on_btnFileClose_clicked();
    void on_btnDAQStart_clicked();
    void on_btnDAQStop_clicked();

    // FEE CITIROC Configuration
    void on_btnCITIROC_Path_clicked();
    void on_btnCITIROC_scFile_clicked();
    void on_btnCITIROC_pbFile_clicked();
    void on_btnReadCITIROC_clicked();
    void on_btnScanConfig2Mem_clicked();
    void on_btnPrintToScreen_clicked();
    void on_btnSendLogic_clicked();
    void on_btnSendConfig_clicked();
    void on_btnSaveCITIROC_clicked();

    // Masks Control
    void on_btnMask_clicked();
    void handle_ScanSpinboxMask();
    void handle_ScanCheckboxMask();
    void on_btnClearMask_clicked();
    void on_btnAllSetMask_clicked();
    void on_btnSignalProbe_clicked();

    // Draw Control
    void on_btnStartDraw_clicked();
    void on_btnStopDraw_clicked();

    // Draw Button
    void on_btnDraw_clicked();
    void handle_DrawOption_Changed();

    void on_btnGenerateIP_clicked();
    void on_btnExit_clicked();
    void on_btnClearDraw_clicked();

    // Window buttons
    void on_btnVISAControl_clicked();
    void on_btnZaberControl_clicked();
};
#define gFEEControlWin (FEEControlWin::Instance())

#endif // WIDGET_H
