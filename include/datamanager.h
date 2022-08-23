#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#define N_BOARD_CHANNELS 32
#define N_SAMPLE_POINTS 50

#include <string>

class TTree;
class TFile;
class FEEControl;
class TH1I;

extern FEEControl *gBoard;

class DataManager
{
public:
    DataManager();
    DataManager(std::string s);
    ~DataManager();

    bool Init(std::string s = "data.root");
    void Close();

    int ProcessFEEData(FEEControl *fee = gBoard);
    inline int GetTotalCount() { return fEventCount; };

    bool IsOpen();     //
    bool Draw(int ch); //
    inline TFile *GetTFile() { return fFile; };
    inline std::string GetFileName() { return sFileName; }

private:
    TFile *fFile = NULL;
    TTree *fTree = NULL;
    std::string sFileName;

    uint32_t *fData = NULL;        // original Data[32*50] for one event, this branch can be delete after check out point processor. Only initiate once while constuction, Init() and Close() will not delete memory
    double fch[N_BOARD_CHANNELS];  // processed data[32] for one event
    TH1I *fHist[N_BOARD_CHANNELS]; // All Histograms

    int fEventCount = 0; // Count how many events is saved

    bool ProcessOneEvent(FEEControl *fee, int &currentIndex); // Process one event in data
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
    bool Draw(int ch);
    bool FitChannel(int ch, double *par = NULL, double *parE = NULL);
    inline std::string GetFileName() { return sFileName; }
    inline TFile *GetTFile() { return fFile; };

private:
    TFile *fFile = NULL;
    TTree *fTree = NULL;
    std::string sFileName = "";
    double fch[N_BOARD_CHANNELS];

    TF1 *fGaus = NULL;
    bool fChFlag = 0;
    double parCh[3], parECh[3];
    int fCh;
};

#define gReadManager (ReadManager::Instance())

#endif // DATAMANAGER_H
