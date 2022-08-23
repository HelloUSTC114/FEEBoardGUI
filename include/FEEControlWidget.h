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

class FEEControlWin : public QWidget
{
    Q_OBJECT

public:
    ~FEEControlWin();
    static FEEControlWin *Instance(); // Forbidden create FEEControlWin by user
    void setParent(QWidget *parent = nullptr) { QWidget::setParent(parent); }

    //! TODO: In the future, this class should adapt to several boards connections.
    bool TryConnect(std::string sIP, int port);

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

private slots:
    // FEE Board Control
    void on_btnConnect_clicked();
    void on_btnHVON_clicked();
    void on_btnHPO_clicked();
    void on_btnHVOFF_clicked();
    void on_btnHVSet_clicked();
    void on_btnRegTest_clicked();
    void on_btnTMon_clicked();

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

    void on_btnMask_clicked();

};
#define gFEEControlWin (FEEControlWin::Instance())

#endif // WIDGET_H
