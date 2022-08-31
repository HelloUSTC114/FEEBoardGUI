#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#define N_BOARD_CHANNELS 32
#define N_SAMPLE_POINTS 50

#include <string>

class TTree;
class TFile;
class FEEControl;
class TH1S;
class TH1I;

class DataManager
{
public:
    DataManager();
    DataManager(std::string s);
    ~DataManager();

    bool Init(std::string s = "data.root");
    void Close();

    /// @brief Process fifo data form FEE board
    /// @param fee to be processed board
    /// @return how many events has been processed in this function
    int ProcessFEEData(FEEControl *fee);
    inline int GetTotalCount() { return fEventCount; };

    bool IsOpen();       //
    bool DrawHG(int ch); // HG Draw
    bool DrawLG(int ch); // LG Draw
    bool DrawTDC(int ch); // TDC Draw
    inline TFile *GetTFile() { return fFile; };
    inline std::string GetFileName() { return sFileName; }

    static double ConvertADC2Amp(uint32_t adc) { return (double)adc / 32.768 - 1000; }; // in unit of mV

private:
    TFile *fFile = NULL;
    TTree *fTree = NULL;
    std::string sFileName;

    uint32_t *fHGData = NULL;  // original HG Data[32*50] for one event, this branch can be delete after check out point processor. Only initiate once while constuction, Init() and Close() will not delete memory
    uint32_t *fLGData = NULL;  // original LG Data[32*50] for one event, this branch can be delete after check out point processor. Only initiate once while constuction, Init() and Close() will not delete memory
    uint32_t *fTDCData = NULL; // original TDC Data[32*50] for one event, this branch can be delete after check out point processor. Only initiate once while constuction, Init() and Close() will not delete memory

    double fHGamp[N_BOARD_CHANNELS];         // processed HG data[32] for one event
    double fLGamp[N_BOARD_CHANNELS];         // processed LG data[32] for one event
    uint32_t fTDCdata[N_BOARD_CHANNELS + 1]; // processed tdc data[32] + 1 time stamp for one event

    TH1S *fHGHist[N_BOARD_CHANNELS];      // All HG Histograms
    TH1S *fLGHist[N_BOARD_CHANNELS];      // All LG Histograms
    TH1I *fTDCHist[N_BOARD_CHANNELS + 1]; // All TDC Histograms

    int fEventCount = 0; // Count how many events is saved

    bool ProcessOneEvent(FEEControl *fee, int &currentIndex); // Process one event in data

    static const int fcgNChannels;     // N channels for one board
    static const int fcgNSamplePoints; // N sample points for one channel in hg/lg
};

extern DataManager *gDataManager;

class TF1;
class ReadManager
{
public:
    ReadManager();
    ReadManager(std::string s);
    ~ReadManager();
    static ReadManager *&Instance();

    bool Init(std::string s = "data.root");
    void Close();

    bool IsOpen();
    bool DrawHG(int ch);
    bool DrawLG(int ch);
    bool DrawTDC(int ch);
    inline std::string GetFileName() { return sFileName; }
    inline TFile *GetTFile() { return fFile; };

private:
    TFile *fFile = NULL;
    TTree *fTree = NULL;
    std::string sFileName = "";
    double fHGamp[N_BOARD_CHANNELS];
    double fLGamp[N_BOARD_CHANNELS];
    uint32_t fTDCdata[N_BOARD_CHANNELS + 1];

    TF1 *fGaus = NULL;
    bool fChFlag = 0;
    double parCh[3], parECh[3];
    int fCh;
};

#define gReadManager (ReadManager::Instance())

#endif // DATAMANAGER_H
