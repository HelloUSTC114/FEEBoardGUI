#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QThread>
#include <QTimer>
#include <QTime>

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
    friend class DeviceMove;
    friend class MoveMonitor;
    friend class DAQRuning;
    friend class FiberTestDrawing;
    friend class ZaberDrawControl;

public:
    ~FEEControlWin();
    static FEEControlWin *Instance(); // Forbidden create FEEControlWin by user
    void setParent(QWidget *parent = nullptr) { QWidget::setParent(parent); }

    //! TODO: In the future, this class should adapt to several boards connections.
    bool TryConnect(std::string sIP, int port);

    /// @brief Try to start DAQ Process
    /// @param sPath SaveFile Folder Path
    /// @param sFileName Save File Name
    /// @param nDAQCount DAQ Count
    /// @param DAQTime DAQ time
    /// @param msBufferSleep FEE board buffer reading waiting time (in ms)
    /// @return whether DAQ start Successfully
    bool TryStartDAQ(std::string sPath, std::string sFileName, int nDAQCount = -1, QTime DAQTime = {0, 0, 0}, int msBufferSleep = 200);
    void TryStartStripTestDAQ(int nCount, QTime daqTime, int msBufferWaiting);

    QDateTime GetFileTimeStamp() { return fFileTimeStamp; };
    std::string GetStripTestInfoFile() { return fsStripFileName; };

    double GetRealCount() { return fDAQRealCount; };
    double GetRealTime() { return fDAQRealTime; };
    double GetCountRate() { return fDAQRealTime == 0 ? 0 : fDAQRealCount / (double)fDAQRealTime * 1000; };

    /// @brief Set Draw Channel in public method
    /// @param channel selected channel
    void SetDrawChannel(int channel);

    /// @brief FEE Logic Selection
    /// @param logic logic number
    void SelectLogic(int logic);

    /// @brief Read, Send, Print to screen , CITIROC configuration file
    /// @param scFile sc configuration file, with path inside
    /// @param pbFile pb configuration file, with path inside
    /// @return whether Read, send, print successfully
    bool RSP_CITIROC_configFile(std::string scFile, std::string pbFile);

    void GetZaberCtrInfo(double &start, double &end, double &step);
    void SetZaberCtrInfo(double start, double end, double step);

signals:
    void startDAQSignal(FEEControlWin *); // FEE Start DAQ signal
    void stopDAQSignal();                 // DAQ Stop signal, tell other class that DAQ is done

private slots:
    void on_DAQStoped(int nDAQLoop);     // FEE DAQ Stop slot
    void handle_ContinousDraw();         // Handle draw slot
    void handle_DAQCount(int nDAQCount); // Handle DAQ Count
    void update_DAQClock();              // Handle DAQ Clock

private:
    /// @brief Generator, private for now. Will be public and update to several boards control
    /// @param parent
    FEEControlWin(QWidget *parent = nullptr);

private:
    Ui::FEEControlWin *ui;

    // FEE Board
    bool fConnected = false; // Whether board is connected

    void PrintConnection(bool flag); // Print Connection information
    void ProcessConnect();           // Process after successfully connected, send logic, send CITIROC config
    void PrintHV();                  // Print HV Information
    void PrintT();                   // Print Temerature information
    void PrintClock();               // Print Si570 clock frequency

    // DAQ control

    /// @brief Force to start DAQ, regardless of file initial, focus only on emit start signal, all start methods should contain this method
    /// @param nCount Count number for DAQ, -1 for forever
    /// @param daqTime Count time for DAQ, 00:00:00.000 for forever
    /// @param msBufferWaiting Parameter for Socket communication, if buffer is not long enough, how long (in ms) should be wait.
    void ForceStartDAQ(int nCount, QTime daqTime, int msBufferWaiting);

    // DAQ File Manager
    bool GenerateROOTFile();  // Generate root file
    QString fsFilePath;       // Save ROOT File Path
    QString fsFileName;       // Save ROOT File Name without time stamp
    QString fsFileNameTotal;  // Save ROOT File Total Name, with time stamp
    QDateTime fFileTimeStamp; //

    int fDAQSettingCount = -1;         // -1 means DAQ forever until stopDAQ clicked
    QTime fDAQSettingTime{0, 0, 0, 0}; // DAQ time setting, 0,0,0,0 means forever until stopDAQ clicked
    int fDAQRealTime = 0;              // monitored real DAQ total time
    int fDAQRealCount = 0;             // Monitored read DAQ count
    volatile bool fDAQRuningFlag = 0;  // DAQ runing flag
    QDateTime fDAQStartTime;           // Record start DAQ time
    QTimer fDAQClock;                  // a 1s clock, aims at updating DAQ time in GUI
    DAQRuning *fDAQProcessor = NULL;   //
    QThread fWorkThread3;              //
    int fDAQBufferSleepms = 200;       // FEE control while DAQ, Wait time of buffer reading

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

    uint32_t fChannelMasks = 0;

    // Fiber Test GUI
    QString sReadFile; //

    // Strip Test GUI
    double fposStart = 0;
    double fposEnd = 0;
    double fstep = 0;
    double fposNow = 0;
    std::string fsStripFileName;

    // Draw Setting
    std::vector<ROOTDraw *> fWinList; // Created ROOTDraw Windows list,
    void DrawSingle();                //
    volatile bool fDrawFlag = 0;      // DAQ Drawing flag
    QTimer fDrawerTimer;              //
    ROOTDraw *fdrawWin = NULL;        //

    // FT analyze window
    FTAnalyzerWin *fFTanaWin = NULL;

private slots:
    // FEE Board Control
    void on_btnConnect_clicked();
    void on_btnPath_clicked();
    void on_btnHVON_clicked();
    void on_btnHPO_clicked();
    void on_btnHVOFF_clicked();
    void on_btnHVSet_clicked();
    void on_btnRegTest_clicked();
    void on_btnTMon_clicked();
    void on_btnFileInit_clicked();
    void on_btnFileClose_clicked();
    void on_btnDAQStart_clicked();
    void on_btnDAQStop_clicked();

    // FEE CITIROC Configuration
    void on_btnToCITIROC_clicked();
    void on_btnCITIROC_Path_clicked();
    void on_btnCITIROC_scFile_clicked();
    void on_btnCITIROC_pbFile_clicked();
    void on_btnReadCITIROC_clicked();
    void on_btnScanConfig2Mem_clicked();
    void on_btnPrintToScreen_clicked();
    void on_btnSendLogic_clicked();
    void on_btnSendConfig_clicked();
    void on_btnSaveCITIROC_clicked();

    // Fiber Test Control
    void on_btnFiberTest_clicked();
    void on_btnFTAnalyzer_clicked();
    void on_btnStopFT_clicked();

    // Draw Control
    void on_btnStartDraw_clicked();
    void on_btnStopDraw_clicked();
    // Draw Button
    void on_btnDraw_clicked();
    // Tab  Control
    void on_btnFiberTab_clicked();

    // Zaber Control Button
    void on_btnZaberWindow_clicked();

    // Strip Test Control
    void on_btnStripTest_clicked();
    void on_btnStopST_clicked();
    void on_btnSTStart_clicked();
    void on_btnZaberPos_clicked();
    void on_btnSTStop_clicked();
    void on_btnMask_clicked();
};
#define gFEEControlWin (FEEControlWin::Instance())

#define gZaberDrawControl (ZaberDrawControl::Instance())
class ZaberDrawControl : public QObject
{
    Q_OBJECT
public:
    static ZaberDrawControl *Instance();
    bool SetZaberPar(double start, double end, double step);
    bool IsOccupied() { return fOccupied; }
    void SetBreak() { fBreakFlag = 1; }

private:
    ZaberDrawControl();
    void ConnectSlots();
    void DisconnectSlots();

    int gHandle = 0;
    double fposStart = 0;
    double fposEnd = 0;
    double fstep = 0;
    double fposSettingNow = 0;
    bool fOccupied = 0;
    volatile bool fBreakFlag = 0;

public slots:
    void startMove();
    void handle_MoveDone(int handle);
    void handle_MoveStart(int handle);

    void handle_DAQStart();
    void handle_DAQDone();
};

#endif // WIDGET_H
