#include "../include/datamanager.h"

#include "feecontrol.h"
#include <TTree.h>
#include <TFile.h>
#include <TH1S.h>
#include <TH1I.h>

using namespace std;

#define CONVERT_ADC_2_AMP(adc) ((double)adc / 2.048 - 1000)
#define CONVERT_AMP_2_ADC(amp) (2.048 * amp + 2048)

double gEventADCThreshold = CONVERT_AMP_2_ADC(100);

DataManager *gDataManager = new DataManager();

DataManager::DataManager()
{
    fHGData = new uint32_t[N_BOARD_CHANNELS * N_SAMPLE_POINTS];
    memset(fHGData, '\0', sizeof(uint32_t) * N_BOARD_CHANNELS * N_SAMPLE_POINTS);
}

DataManager::DataManager(string sInput) : DataManager()
{
    sFileName = sInput;
    Init(sInput);
}

DataManager::~DataManager()
{
    Close();
    delete[] fHGData;
    fHGData = NULL;
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
        fHGHist[i] = new TH1S(Form("hHG%d", i), Form("High Gain ch%d", i), 2047, 2048, 4095);
        fLGHist[i] = new TH1S(Form("hLG%d", i), Form("Low Gain ch%d", i), 2047, 2048, 4095);
        fTDCHist[i] = new TH1I(Form("hTDC%d", i), Form("TDC Value ch%d", i), 2 ^ 16, 0, 2 ^ 32);
    }
    fTDCHist[32] = new TH1I(Form("hTDC%d", 32), Form("TDC Value ch%d", 32), 2 ^ 16, 0, 2 ^ 32);

    // fTree->Branch("data", fHGData, "data[1600]/i");
    fTree->Branch("chHG", fHGamp, "chHG[32]/D");
    fTree->Branch("chLG", fLGamp, "chLG[32]/D");
    fTree->Branch("chTDC", fTDCdata, "chLG[33]/I");
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

    memset(fHGHist, '\0', N_BOARD_CHANNELS * sizeof(TH1S *));

    fEventCount = 0;
}

bool DataManager::DrawHG(int ch)
{
    if (ch < 0 || ch > 31)
        return false;
    if (!IsOpen() || !fTree)
    {
        return false;
    }
    fHGHist[ch]->Draw();
    return true;
}

bool DataManager::DrawLG(int ch)
{
    if (ch < 0 || ch > 31)
        return false;

    if (!IsOpen() || !fTree)
    {
        return false;
    }
    fLGHist[ch]->Draw();
    return true;
}

bool DataManager::DrawTDC(int ch)
{
    if (ch < 0 || ch > 32)
        return false;

    if (!IsOpen() || !fTree)
    {
        return false;
    }
    fTDCHist[ch]->Draw();
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
    auto dat = fee->GetTestFIFOData();
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
        fHGamp[ch] = 0;
        addCounter = 0;
        for (int point = 0; point < N_SAMPLE_POINTS; point++)
        {
            if (point >= 5 && point < 46)
            {
                double sampleValue = dat[startPoint + ch * N_SAMPLE_POINTS + point];
                // fHGData[ch * N_SAMPLE_POINTS + point] = sampleValue;
                fHGamp[ch] += sampleValue;
                addCounter++;
            }
        }
        // fHGamp is average of these points
        fHGamp[ch] /= addCounter;
    }

    // end of the event, counter increment
    currentIndex += N_BOARD_CHANNELS * N_SAMPLE_POINTS;
    fEventCount++;
    fTree->Fill();
    for (int ch = 0; ch < N_BOARD_CHANNELS; ch++)
    {
        fHGHist[ch]->Fill(fHGamp[ch]);
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

ReadManager::ReadManager(string sInput) : ReadManager()
{
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

    // fTree->Branch("data", fHGData, "data[1600]/i");
    fTree->SetBranchAddress("ch", fHGamp);
    fTree->SetBranchAddress("chHG", fHGamp);
    fTree->SetBranchAddress("chLG", fLGamp);
    fTree->SetBranchAddress("chTDC", fTDCdata);
    return true;
}

ReadManager *&ReadManager::Instance()
{
    static ReadManager *fInstance = new ReadManager();
    return fInstance;
}

bool ReadManager::DrawHG(int ch)
{
    if (ch < 0 || ch > 31)
        return false;

    if (!IsOpen() || !fTree)
    {
        // Init("F:/Projects/MuonTestControl/Data/Fiber-00.root");
        return false;
    }

    auto hHG = (TH1F *)fFile->Get(Form("hHG%d", ch));
    if (hHG)
    {
        hHG->Draw();
        return true;
    }
    fFile->cd();
    fTree->Draw(Form("chHG[%d]>>hHG%d(2047, 2048, 4095)", ch, ch));
    hHG = (TH1F *)fFile->Get(Form("hHG%d", ch));

    if (!hHG)
        return false;

    hHG->SetTitle(Form("HG Draw channel:%d;ADC Value; Counts", ch));
    return true;
}

bool ReadManager::DrawLG(int ch)
{
    if (ch < 0 || ch > 31)
        return false;

    if (!IsOpen() || !fTree)
    {
        // Init("F:/Projects/MuonTestControl/Data/Fiber-00.root");
        return false;
    }

    auto hLG = (TH1F *)fFile->Get(Form("hLG%d", ch));
    if (hLG)
    {
        hLG->Draw();
        return true;
    }
    fFile->cd();
    fTree->Draw(Form("chLG[%d]>>hLG%d(2047, 2048, 4095)", ch, ch));
    hLG = (TH1F *)fFile->Get(Form("hLG%d", ch));

    if (!hLG)
        return false;

    hLG->SetTitle(Form("LG Draw channel:%d;ADC Value; Counts", ch));
    return true;
}

bool ReadManager::DrawTDC(int ch)
{
    if (ch < 0 || ch > 32)
        return false;
    if (!IsOpen() || !fTree)
    {
        // Init("F:/Projects/MuonTestControl/Data/Fiber-00.root");
        return false;
    }

    auto hTDC = (TH1F *)fFile->Get(Form("hTDC%d", ch));
    if (hTDC)
    {
        hTDC->Draw();
        return true;
    }
    fFile->cd();
    fTree->Draw(Form("chTDC[%d]>>hTDC%d(2047, 2048, 4095)", ch, ch));
    hTDC = (TH1F *)fFile->Get(Form("hTDC%d", ch));

    if (!hTDC)
        return false;

    hTDC->SetTitle(Form("TDC Draw channel:%d;ADC Value; Counts", ch));
    return true;
}
