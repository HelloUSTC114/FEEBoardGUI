#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#define N_BOARD_CHANNELS 32
#define N_SAMPLE_POINTS 40

#include <string>
#include <QDateTime>

class TTree;
class TBranch;
class TFile;
class FEEControl;
class TH1S;
class TH1I;
class TDatime;
class TString;

//! \enum define which data should be drawn, usful only in class DataManager Draw function, and class ReadManager Draw function
enum DrawOption
{
    HGDataDraw = 0,
    LGDataDraw,
    TDCDataDraw
};

class DataManager
{
public:
    DataManager();
    DataManager(std::string s);
    ~DataManager();

    bool Init(std::string s = "data.root");
    void Close();

    bool SetBoardNo(int boardno);
    bool SetDAQDatime(const QDateTime &inputDT);
    bool SetDAQTemp(const double *tempArray);
    bool SetCITIROCConfig(std::string sConfig);
    bool SetSelectedLogic(int logic);

    /// @brief Process fifo data form FEE board
    /// @param fee to be processed board
    /// @return how many events has been processed in this function, only return HG data count
    int ProcessFEEData(FEEControl *fee);

    inline int GetTotalCount() { return fEventCount; };
    inline int GetHGTotalCount() { return fHGEventCount; };
    inline int GetLGTotalCount() { return fLGEventCount; };
    inline int GetTDCTotalCount() { return fTDCEventCount; };

    bool IsOpen(); //

    bool Draw(int ch, DrawOption option); // control draw through drawOption
    bool DrawHG(int ch);                  // HG Draw
    bool DrawLG(int ch);                  // LG Draw
    bool DrawTDC(int ch);                 // TDC Draw
    void ClearDraw();                     // Clear All chs inside memory
    inline TFile *GetTFile() { return fFile; };
    inline std::string GetFileName() { return sFileName; }

    static double GetFreq() { return fgFreq; };
    static double ConvertADC2Amp(uint16_t adc) { return (double)adc / 32.768 - 1000; };                             // in unit of mV
    static double ConvertTDC2Time(uint64_t tdc) { return ((tdc >> 16) - (tdc & 0xffff) / 65536.0) / fgFreq * 1e3; } // in unit of ns
    static double ConvertTDC2Time(uint64_t tdc, double &coarseTime, double &fineTime);                              // in unit of ns

private:
public:
    // Event Head
    TDatime *fDAQDatime = NULL;
    bool fDatimeFlag = 0;
    TString *fTemp[4]{0, 0, 0, 0};
    bool fTFlag = 0;
    TString *fBoardNo = NULL;
    bool fBoardFlag = 0;
    TString *fConfig = NULL;
    bool fConfigFlag = 0;
    TString *fSelectedLogic = NULL;
    bool fLogicFlag = 0;

    // Event Body
    TFile *fFile = NULL;
    TTree *fTree = NULL;
    TTree *fTrees[3]{NULL, NULL, NULL};
    TTree *&fHGTree = fTrees[0];
    TTree *&fLGTree = fTrees[1];
    TTree *&fTDCTree = fTrees[2];

    std::string sFileName;

    uint32_t *fPreviousData = NULL; // original HG Data[32*50] for one event, this branch can be delete after check out point processor. Only initiate once while constuction, Init() and Close() will not delete memory

    // Save variables
    uint32_t fADCid[2]{0, 0};
    uint32_t &fHGid = fADCid[0], &fLGid = fADCid[1], fTDCid = 0;
    double fHGamp[N_BOARD_CHANNELS];          // processed HG data[32] for one event
    double fLGamp[N_BOARD_CHANNELS];          // processed LG data[32] for one event
    double *const fADCamp[2]{fHGamp, fLGamp}; // Merge ADC data
    uint64_t fTDCTime[N_BOARD_CHANNELS + 1];  // processed tdc data[32] + 1 time stamp for one event

    TH1S *fHGHist[N_BOARD_CHANNELS]{NULL};      // All HG Histograms
    TH1S *fLGHist[N_BOARD_CHANNELS]{NULL};      // All LG Histograms
    TH1S **const fADCHist[2]{fHGHist, fLGHist}; // ADC Histograms
    TH1I *fTDCHist[N_BOARD_CHANNELS + 1];       // All TDC Histograms

    int fEventCount = 0; // Count how many events has processed
    int fADCEventCount[2]{0, 0};
    int &fHGEventCount = fADCEventCount[0]; // Count how many events has processed
    int &fLGEventCount = fADCEventCount[1]; // Count how many events has processed
    int fTDCEventCount = 0;                 // Count how many events has processed

    int fADCLastLoopEventCount[2]{0, 0};
    int &fHGLastLoopEventCount = fADCLastLoopEventCount[0]; // Monitor how many events has processed in last events processing function
    int &fLGLastLoopEventCount = fADCLastLoopEventCount[1]; // Monitor how many events has processed in last events processing function
    int fTDCLastLoopEventCount = 0;                         // Monitor how many events has processed in last events processing function

    // Buffer declaration
    int fADCBufCount[2]{0, 0};
    int &fHGBufCount = fADCBufCount[0];
    int &fLGBufCount = fADCBufCount[1];
    int fTDCBufCount = 0; // Left point in Buffer
    uint16_t *fADCBuffer[2]{NULL, NULL};
    uint16_t *&fHGBuffer = fADCBuffer[0]; // HG Data buffer for unprocessed data. Only initiate once while constuction, Init() and Close() will not delete memory
    uint16_t *&fLGBuffer = fADCBuffer[1]; // LG Data buffer for unprocessed data. Only initiate once while constuction, Init() and Close() will not delete memory
    uint16_t *fTDCBuffer = NULL;          // TDC Data buffer for unprocessed data. Only initiate once while constuction, Init() and Close() will not delete memory
    void ClearBuffer();                   // Only set counter to zero, no need to absolute set to zero

    // Start Processing
    FEEControl *fFEECurrentProcessing;    // Processing Board pointer
    int fADCHeadIndex[2]{0, 0};           // Head indexs for adc
    int &fHGHeadIndex = fADCHeadIndex[0]; // Head index
    int &fLGHeadIndex = fADCHeadIndex[1]; // Head index
    int fTDCHeadIndex = 0;                // Head index for tdc

    int ProcessADCEvents(int adcNo);                                           // Process HG&LG adc data for one event, 0 for HG, 1 for LG
    int ProcessADCEvents(int adcNo, const uint16_t *src_data, int dataLength); // Process HG&LG adc data, for test
    int ProcessTDCEvents();
    int ProcessTDCEvents(const uint16_t *src_data, int dataLength);
    // int ProcessADCEvents(int adcNo, const uint32_t *src_data, int dataLength); // Process HG&LG adc data, for test
    // int ProcessTDCEvents(const uint32_t *src_data, int dataLength);

    void FillADCData(int adcNo);
    void FillHGData();
    void FillLGData();
    void FillTDCData();

    /// @brief Process data from HG, LG adc in one event. First judge pedestal value, then give front-edge threshold.
    // Meanwhile, judge whether data is valid, through:
    //  -- whether channel mean value is under threshold
    //  -- whether find invalid data 65535
    //  -- signal time should be
    /// @param iter_first  [in] start iterator, must be first head
    /// @param iter_end [in] end iterator, at least large than point factor, usually is set as last data in fifo, ptr_data + data_length - 1
    /// @param id   [out] trigger id
    /// @param chArray   [out] calculated amp value save array
    /// @param dataLength   [out] length from head to data end, useful for next head searching
    /// @return whether this data is valid
    bool ProcessOneADCEvent(const uint16_t *const iter_first, const uint16_t *const iter_end, uint32_t &id, double *chArray, int &dataLength);

    /// @brief Process data from TDC
    /// @param iter_first
    /// @param iter_end
    /// @param dataLength [out] data[head+dataLength] is another head. if datalength is 0, means cannot find the next header, error
    /// @return
    bool ProcessOneTDCEvent(const uint16_t *const iter_first, const uint16_t *const iter_end, int &dataLength);

    // will be deleted later
    bool ProcessOneEvent(FEEControl *fee, int &currentIndex); // Process one event in data

    // public variables
    static const int fcgNChannels;     // N channels for one board
    static const int fcgNSamplePoints; // N sample points for one channel in hg/lg
    static double fgFreq;              // frequency MHz
    static int fADCLengthFactor;       // adc Length Factor
    static int fTDCLengthFactor;       // tdc Length Factor

    // Test functions
    void PrintHGBuffer();
    void PrintLGBuffer();
    void PrintTDCBuffer();
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
    bool Draw(int ch, DrawOption option);
    bool DrawHG(int ch);
    bool DrawLG(int ch);
    bool DrawTDC(int ch);
    inline std::string GetFileName() { return sFileName; }
    inline TFile *GetTFile() { return fFile; };

private:
    TFile *fFile = NULL;
    TTree *fTree = NULL;
    TTree *fHGTree = NULL;
    TTree *fLGTree = NULL;
    TTree *fTDCTree = NULL;

    std::string sFileName = "";
    double fHGamp[N_BOARD_CHANNELS];
    double fLGamp[N_BOARD_CHANNELS];
    uint64_t fTDCTime[N_BOARD_CHANNELS + 1];

    TF1 *fGaus = NULL;
    bool fChFlag = 0;
    double parCh[3], parECh[3];
    int fCh;
};

#define gReadManager (ReadManager::Instance())

#endif // DATAMANAGER_H
