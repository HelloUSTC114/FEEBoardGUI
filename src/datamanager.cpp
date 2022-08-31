#include "../include/datamanager.h"

#include "feecontrol.h"
#include <TTree.h>
#include <TFile.h>
#include <TH1I.h>

using namespace std;

#define CONVERT_ADC_2_AMP(adc) ((double)adc / 2.048 - 1000)
#define CONVERT_AMP_2_ADC(amp) (2.048 * amp + 2048)

double gEventADCThreshold = CONVERT_AMP_2_ADC(100);

DataManager *gDataManager = new DataManager();

DataManager::DataManager()
{
    fData = new uint32_t[N_BOARD_CHANNELS * N_SAMPLE_POINTS];
    memset(fData, '\0', sizeof(uint32_t) * N_BOARD_CHANNELS * N_SAMPLE_POINTS);
}

DataManager::DataManager(string sInput) : sFileName(sInput)
{
    fData = new uint32_t[N_BOARD_CHANNELS * N_SAMPLE_POINTS];
    memset(fData, '\0', sizeof(uint32_t) * N_BOARD_CHANNELS * N_SAMPLE_POINTS);
    Init(sInput);
}

DataManager::~DataManager()
{
    Close();
    delete[] fData;
    fData = NULL;
}

bool DataManager::IsOpen()
{
    if (!fFile)
        return false;
    return fFile->IsOpen();
}

bool DataManager::Init(string sInput)
{
    if (fFile)
        Close();

    sFileName = sInput;

    fFile = new TFile(sFileName.c_str(), "recreate");
    if (!fFile->IsOpen())
        return false;
    fTree = new TTree("fifo", "Fifo Data");

    for (int i = 0; i < N_BOARD_CHANNELS; i++)
    {
        fHist[i] = new TH1I(Form("h%d", i), Form("h%d", i), 2047, 2048, 4095);
    }

    // fTree->Branch("data", fData, "data[1600]/i");
    fTree->Branch("ch", fch, "ch[32]/D");
    fTree->AutoSave();
    fEventCount = 0;

    return true;
}

void DataManager::Close()
{
    fFile->cd();
    if (fTree && fFile && fFile->IsWritable())
    {
        fTree->Write();
    }
    fFile->Close();

    delete fFile;
    fFile = NULL;
    fTree = NULL;

    memset(fHist, '\0', N_BOARD_CHANNELS * sizeof(TH1I *));

    fEventCount = 0;
}

bool DataManager::Draw(int ch)
{
    if (!IsOpen() || !fTree)
    {
        return false;
    }
    fHist[ch]->Draw();
    return true;
}

int DataManager::ProcessFEEData(FEEControl *fee)
{
    if (fee == NULL || !fee->GetDataValidation())
    {
        return 0;
    }

    int eventCounter = 0;
    int currentIndex = 0;
    while (ProcessOneEvent(fee, currentIndex))
    {
        eventCounter++;
    }

    return eventCounter;
}

bool DataManager::ProcessOneEvent(FEEControl *fee, int &currentIndex)
{
    auto dat = fee->GetFifoData();
    // dealing with unexpected case, where the first point lies beyond threshold
    if (dat[currentIndex] >= gEventADCThreshold)
    {
        for (; currentIndex < fee->GetDataLength();)
        {
            if (dat[currentIndex++] < gEventADCThreshold)
                break;
        }
    }

    // Judge when is data beyond threshold
    bool loopFlag = 0;
    for (; currentIndex < fee->GetDataLength();)
    {
        // Judge whether data is beyond threshols, set startpoint as start point of the event.
        if (dat[currentIndex++] >= gEventADCThreshold)
        {
            // Judge whether arrive end of array
            if (currentIndex + N_BOARD_CHANNELS * N_SAMPLE_POINTS >= fee->GetDataLength())
                return false;

            loopFlag = 1;
            currentIndex--;
            break;
        }
    }
    if (!loopFlag)
        return false;

    int startPoint = currentIndex;

    int addCounter = 0; // stat how many points are added together
    for (int ch = 0; ch < N_BOARD_CHANNELS; ch++)
    {
        fch[ch] = 0;
        addCounter = 0;
        for (int point = 0; point < N_SAMPLE_POINTS; point++)
        {
            if (point >= 5 && point < 46)
            {
                double sampleValue = dat[startPoint + ch * N_SAMPLE_POINTS + point];
                // fData[ch * N_SAMPLE_POINTS + point] = sampleValue;
                fch[ch] += sampleValue;
                addCounter++;
            }
        }
        // fch is average of these points
        fch[ch] /= addCounter;
    }

    // end of the event, counter increment
    currentIndex += N_BOARD_CHANNELS * N_SAMPLE_POINTS;
    fEventCount++;
    fTree->Fill();
    for (int ch = 0; ch < N_BOARD_CHANNELS; ch++)
    {
        fHist[ch]->Fill(fch[ch]);
    }
    return true;
}

#include <TH1D.h>
#include <iostream>
#include <time.h>
#include <TF1.h>

ReadManager::ReadManager()
{
    fGaus = new TF1("fGausFTAnalyzer", "gaus", 0, 4096);
}

ReadManager::ReadManager(string sInput)
{
    fGaus = new TF1("fGausFTAnalyzer", "gaus", 0, 4096);
    Init(sInput);
}

ReadManager::~ReadManager()
{
    Close();
}

void ReadManager::Close()
{
    delete fGaus;
    fGaus = NULL;

    if (fFile)
        fFile->Close();
    delete fFile;
    fFile = NULL;
    fTree = NULL;
}

bool ReadManager::IsOpen()
{
    if (!fFile)
        return false;
    return fFile->IsOpen();
}

bool ReadManager::Init(string sInput)
{
    if (fFile)
        Close();

    sFileName = sInput;

    fFile = new TFile(sFileName.c_str());
    if (!fFile->IsOpen())
        return false;
    fTree = (TTree *)fFile->Get("fifo");

    // fTree->Branch("data", fData, "data[1600]/i");
    fTree->SetBranchAddress("ch", fch);

    return true;
}

ReadManager *&ReadManager::Instance()
{
    static ReadManager *fInstance = new ReadManager();
    return fInstance;
}

bool ReadManager::Draw(int ch)
{

    if (!IsOpen() || !fTree)
    {
        // return false;
        Init("F:/Projects/MuonTestControl/Data/Fiber-00.root");
    }

    auto h = (TH1F *)fFile->Get(Form("h%d", ch));
    if (h)
    {
        h->Draw();
        return true;
    }
    fFile->cd();
    fTree->Draw(Form("ch[%d]>>h%d(2047, 2048, 4095)", ch, ch));
    h = (TH1F *)fFile->Get(Form("h%d", ch));

    if (!h)
        return false;

    h->SetTitle(Form("Draw channel:%d;ADC Value; Counts", ch));
    return true;
}

bool ReadManager::FitChannel(int ch, double *par, double *parE)
{
    if (!fFile)
    {
        cout << "Not Open file" << endl;
        return false;
    }
    auto h = (TH1F *)fFile->Get(Form("h%d", ch));
    if (!h)
    {
        auto rtn = this->ReadManager::Draw(ch);
        cout << rtn << endl;
        if (!rtn)
        {
            return false;
        }
        fFile->ls();
    }
    h = (TH1F *)fFile->Get(Form("h%d", ch));
    if (!h)
        return false;

    h->Fit(fGaus, "RQ");
    for (int i = 0; i < 3; i++)
    {
        parCh[i] = fGaus->GetParameter(i);
        parECh[i] = fGaus->GetParError(i);
        if (par)
            par[i] = parCh[i];
        if (parE)
            parE[i] = parECh[i];
    }

    fChFlag = 1;
    fCh = ch;
    return true;
}
