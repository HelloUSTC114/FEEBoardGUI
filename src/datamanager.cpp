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
int DataManager::fgFreq = 1; // in unit of MHz
int DataManager::fADCPointFactor = FEEControl::fHGPointFactor;
int DataManager::fTDCPointFactor = FEEControl::fTDCPointFactor;

DataManager *gDataManager = new DataManager();

DataManager::DataManager()
{
    fPreviousData = new uint32_t[N_BOARD_CHANNELS * N_SAMPLE_POINTS];
    memset(fPreviousData, '\0', sizeof(uint32_t) * N_BOARD_CHANNELS * N_SAMPLE_POINTS);

    fHGBuffer = new uint32_t[(int)(2.5 * fADCPointFactor)];
    fLGBuffer = new uint32_t[(int)(2.5 * fADCPointFactor)];
    fTDCBuffer = new uint32_t[(int)(2.5 * fTDCPointFactor)];
    memset(fHGBuffer, '\0', sizeof(uint32_t) * (int)(2.5 * fADCPointFactor));
    memset(fLGBuffer, '\0', sizeof(uint32_t) * (int)(2.5 * fADCPointFactor));
    memset(fTDCBuffer, '\0', sizeof(uint32_t) * (int)(2.5 * fTDCPointFactor));
}

DataManager::DataManager(string sInput) : DataManager()
{
    sFileName = sInput;
    Init(sInput);
}

DataManager::~DataManager()
{
    Close();
    delete[] fPreviousData;
    fPreviousData = NULL;

    delete[] fHGBuffer;
    fHGBuffer = NULL;
    delete[] fLGBuffer;
    fLGBuffer = NULL;
    delete[] fTDCBuffer;
    fTDCBuffer = NULL;
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
    fHGTree = new TTree("HGTree", "HG Data");
    fLGTree = new TTree("LGTree", "LG Data");
    fTDCTree = new TTree("TDCTree", "TDC Data");

    for (int i = 0; i < N_BOARD_CHANNELS; i++)
    {
        fHGHist[i] = new TH1S(Form("hHG%d", i), Form("High Gain ch%d", i), 2047, 2048, 4095);
        fLGHist[i] = new TH1S(Form("hLG%d", i), Form("Low Gain ch%d", i), 2047, 2048, 4095);
        fTDCHist[i] = new TH1I(Form("hTDC%d", i), Form("TDC Value ch%d", i), 2 ^ 16, 0, 2 ^ 32);
    }
    fTDCHist[32] = new TH1I(Form("hTDC%d", 32), Form("TDC Value ch%d", 32), 2 ^ 16, 0, 2 ^ 32);

    // fTree->Branch("data", fPreviousData, "data[1600]/i");
    fHGTree->Branch("chHG", fHGamp, "chHG[32]/D");
    fHGTree->Branch("chHGid", &fHGid, "chHGid/l");
    fLGTree->Branch("chLG", fLGamp, "chLG[32]/D");
    fLGTree->Branch("chLGid", &fLGid, "chLGid/l");
    fTDCTree->Branch("chTDC", fTDCtime, "chLG[33]/l");
    fTDCTree->Branch("chTDCid", &fTDCid, "chTDCid/l");

    // fTree->AutoSave();
    fHGTree->AutoSave();
    fLGTree->AutoSave();
    fTDCTree->AutoSave();
    ClearBuffer();
    return true;
}

void DataManager::Close()
{
    fFile->cd();
    if (fHGTree && fFile && fFile->IsWritable())
    {
        fHGTree->Write();
        fLGTree->Write();
        fTDCTree->Write();
    }
    fFile->Close();

    delete fFile;
    fFile = NULL;
    fHGTree = NULL;
    fLGTree = NULL;
    fTDCTree = NULL;

    memset(fHGHist, '\0', N_BOARD_CHANNELS * sizeof(TH1S *));
    memset(fLGHist, '\0', N_BOARD_CHANNELS * sizeof(TH1S *));
    memset(fTDCHist, '\0', N_BOARD_CHANNELS * sizeof(TH1S *));

    ClearBuffer();
}

bool DataManager::DrawHG(int ch)
{
    if (ch < 0 || ch > 31)
        return false;
    if (!IsOpen() || !fHGTree)
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

    if (!IsOpen() || !fLGTree)
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

    if (!IsOpen() || !fTDCTree)
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

void DataManager::ClearBuffer()
{
    fEventCount = 0;
    fHGBufCount = fLGBufCount = fTDCBufCount = 0;
    fHGEventCount = fLGEventCount = fTDCEventCount = 0;
}

bool DataManager::ProcessOneEvent(FEEControl *fee, int &currentIndex)
{
    auto dat = fee->GetTestFIFOData();
    // dealing with unexpected case, where the first point lies beyond threshold
    if (dat[currentIndex] >= gEventADCThreshold)
    {
        for (; currentIndex < fee->GetTestDataLength();)
        {
            if (dat[currentIndex++] < gEventADCThreshold)
                break;
        }
    }

    // Judge when is data beyond threshold
    bool loopFlag = 0;
    for (; currentIndex < fee->GetTestDataLength();)
    {
        // Judge whether data is beyond threshols, set startpoint as start point of the event.
        if (dat[currentIndex++] >= gEventADCThreshold)
        {
            // Judge whether arrive end of array
            if (currentIndex + N_BOARD_CHANNELS * N_SAMPLE_POINTS >= fee->GetTestDataLength())
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
                // fPreviousData[ch * N_SAMPLE_POINTS + point] = sampleValue;
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
    // fTree->Fill();
    for (int ch = 0; ch < N_BOARD_CHANNELS; ch++)
    {
        fHGHist[ch]->Fill(fHGamp[ch]);
    }
    return true;
}

int DataManager::ProcessADCEvents(int adcNo)
{
    if (adcNo != 0 && adcNo != 1)
        return -1;

    const uint32_t *adcdata = NULL;
    int adcdatalength = 0;
    if (adcNo == 0)
    {
        adcdata = fFEECurrentProcessing->GetHGFIFOData();
        adcdatalength = fFEECurrentProcessing->GetHGDataLength();
    }
    else if (adcNo == 1)
    {
        adcdata = fFEECurrentProcessing->GetLGFIFOData();
        adcdatalength = fFEECurrentProcessing->GetLGDataLength();
    }

    int headFirst = 0, headLast = 0, totalEventCounter = 0;

    // if there're points left in buffer, concatenate this new data with buffer inside
    if (fADCBufCount[adcNo] > 0)
    {
        // Copy the first left event into buffer
        // -- search first head
        for (int idx_search = 0; idx_search < fADCPointFactor; idx_search++)
        {
            if (adcdata[idx_search] == 65535 && adcdata[idx_search + 1] == 65535)
            {
                headFirst = idx_search;
                break;
            }
        }
        memcpy(fADCBuffer[adcNo], adcdata, headFirst * sizeof(uint32_t));
        fADCBufCount[adcNo] += headFirst;

        int idx_processed = 0;
        bool dataflag = ProcessOneADCEvent(fADCBuffer[adcNo], fADCBuffer[adcNo] + fADCBufCount[adcNo], fADCid[adcNo], fADCamp[adcNo], idx_processed);
        if (dataflag)
        {
            // Fill Data
            FillADCData(adcNo);
            totalEventCounter++;
            fADCEventCount[adcNo]++;
        }

        // Judge whether contains 2 heads
        int headInside = 0;
        for (int idx_search = idx_processed > 2 ? idx_processed : 2; idx_search < fADCBufCount[adcNo]; idx_search++)
        {
            if (adcdata[idx_search] == 65535 && adcdata[idx_search + 1] == 65535)
            {
                headInside = idx_search;
                break;
            }
        }
        if (headInside > 0)
        {
            idx_processed = 0;
            bool dataflag = ProcessOneADCEvent(fADCBuffer[adcNo], fADCBuffer[adcNo] + fADCBufCount[adcNo], fADCid[adcNo], fADCamp[adcNo], idx_processed);
            if (dataflag)
            {
                // Fill Data
                FillADCData(adcNo);
                totalEventCounter++;
                fADCEventCount[adcNo]++;
            }
        }
        fADCBufCount[adcNo] = 0;
    }

    // Watch out, here, if (head1, head2, id1, id2) is (65535, 65535,65535, 65535), the first head1, head2 will be recongnized normally
    for (int idx_search = headFirst; idx_search < adcdatalength - 1; idx_search++)
    {
        bool flagHead = adcdata[idx_search] == 65535 && adcdata[idx_search + 1] == 65535;
        if (!flagHead)
            continue;

        int head = idx_search;

        // if this event is the last, copy memory to buffer, and record buffer length, then break this process loop
        if (head + fADCPointFactor > adcdatalength - 10)
        {
            fADCBufCount[adcNo] = adcdatalength - head;
            memcpy(fADCBuffer[adcNo], adcdata + head, (adcdatalength - head) * sizeof(uint32_t));
            break;
        }

        int idx_processed = 0;
        bool dataflag = ProcessOneADCEvent(adcdata + head, adcdata + adcdatalength, fADCid[adcNo], fADCamp[adcNo], idx_processed);

        if (!dataflag)
        {
            if (idx_processed == 0)
                return totalEventCounter;
            else
            {
                idx_search += idx_processed;
                continue;
            }
        }

        // Fill Data
        FillADCData(adcNo);
        totalEventCounter++;
        fADCEventCount[adcNo]++;

        idx_search += idx_processed - 10; // skip data points, so as to save plenty of searching time
    }

    return totalEventCounter;
}

int DataManager::ProcessTDCEvents()
{
    const uint32_t *tdcdata = fFEECurrentProcessing->GetTDCFIFOData();
    int tdcdatalength = fFEECurrentProcessing->GetTDCDataLength();

    int headFirst = 0, headLast = 0, totalEventCounter = 0;

    // if there're points left in buffer, concatenate this new data with buffer inside
    if (fTDCBufCount > 0)
    {
        // Copy the first left event into buffer
        // -- search first head
        for (int idx_search = 0; idx_search < fTDCPointFactor; idx_search++)
        {
            if (tdcdata[idx_search] == 65535 && tdcdata[idx_search + 1] == 65535)
            {
                headFirst = idx_search;
                break;
            }
        }
        memcpy(fTDCBuffer, tdcdata, headFirst * sizeof(uint32_t));
        fTDCBufCount += headFirst;

        int idx_processed = 0;
        bool dataflag = ProcessOneTDCEvent(fTDCBuffer, fTDCBuffer + fTDCBufCount, idx_processed);
        if (dataflag)
        {
            // Fill Data
            FillTDCData();
            totalEventCounter++;
            fTDCEventCount++;
        }
        fTDCBufCount = 0;
    }

    // Watch out, here, if (head1, head2, id1, id2) is (65535, 65535,65535, 65535), the first head1, head2 will be recongnized normally
    for (int idx_search = headFirst; idx_search < tdcdatalength - 1; idx_search++)
    {
        bool flagHead = tdcdata[idx_search] == 65535 && tdcdata[idx_search + 1] == 65535;
        if (!flagHead)
            continue;

        int head = idx_search;

        // if this event is the last, copy memory to buffer, and record buffer length, then break this process loop
        if (head + fTDCPointFactor > tdcdatalength)
        {
            fTDCBufCount = tdcdatalength - head;
            memcpy(fTDCBuffer, tdcdata + head, (tdcdatalength - head) * sizeof(uint32_t));
            break;
        }

        int idx_processed = 0;
        bool dataflag = ProcessOneTDCEvent(tdcdata + head, tdcdata + tdcdatalength, idx_processed);

        if (!dataflag)
        {
            if (idx_processed == 0)
                return totalEventCounter;
            else
            {
                idx_search += idx_processed;
                continue;
            }
        }

        // Fill Data
        FillTDCData();
        totalEventCounter++;
        fTDCEventCount++;

        idx_search += idx_processed - 1; // skip data points, so as to save plenty of searching time
    }

    // The TDC unique situation: (no check for double heads inside buffer)
    if (tdcdata[tdcdatalength - 1] == 65535 && fTDCBufCount == 0)
    {
        fTDCBufCount = 1;
        fTDCBuffer[0] = 65535;
    }

    return totalEventCounter;
}

void DataManager::FillADCData(int adcNo)
{
    if (adcNo == 0)
        FillHGData();
    else if (adcNo == 1)
        FillLGData();
}

void DataManager::FillHGData()
{
    fHGTree->Fill();
    for (int ch = 0; ch < 32; ch++)
    {
        fHGHist[ch]->Fill(fHGamp[ch]);
    }
}

void DataManager::FillLGData()
{
    fLGTree->Fill();
    for (int ch = 0; ch < 32; ch++)
    {
        fLGHist[ch]->Fill(fLGamp[ch]);
    }
}

#include <TMath.h>
bool DataManager::ProcessOneADCEvent(const uint32_t *const iter_first, const uint32_t *const iter_end, uint32_t &id, double *chArray, int &idx_processed)
{
    // Process trigger id
    id = (*(iter_first + 2) << 16) + (*(iter_first + 3) & 0xffff);

    // thrshold Calculating
    double ped = TMath::Mean(iter_first + 5, iter_first + 25);
    double pedrms = TMath::StdDev(iter_first + 5, iter_first + 25);
    double thr_signal = ped + pedrms * 10; // set threshold at pedestal + 10 sigma

    // Find start data
    int idx_ch0 = 0;
    for (auto idx_search = 0; idx_search < (int)(iter_end - iter_first); idx_search++)
    {
        if (*(iter_first + idx_search) > thr_signal)
        {
            idx_ch0 = idx_search;
            break;
        }
    }
    if (idx_ch0 == 0)
    {
        idx_processed = 0;
        return false;
    }

    // id = (*);
    // id = (hgdata[head + 2] << 16) + (hgdata[head + 3] & 0xffff);
    for (int ch = 0; ch < N_BOARD_CHANNELS; ch++)
    {
        auto iterFirst = iter_first + idx_ch0 + N_SAMPLE_POINTS * ch;
        auto iterEnd = iter_first + idx_ch0 + N_SAMPLE_POINTS * ch + N_SAMPLE_POINTS - 1;
        int boardLength = 2; // abort edge points
        chArray[ch] = TMath::Mean(iterFirst + boardLength, iterEnd - boardLength);

        // data error processing, if one channel is beyond pedestal, it may reveals data error, and this will be a invalid data.
        if (chArray[ch] < thr_signal)
        {
            idx_processed = idx_ch0 + N_SAMPLE_POINTS * (ch - 1 > 0 ? ch - 1 : 0);
            return false;
        }
    }
    // In case of all 31 channels are beyond threshold, but the last few points were set into below pedestal
    if (*(iter_first + idx_ch0 + N_SAMPLE_POINTS * N_BOARD_CHANNELS - 3) < thr_signal)
    {
        idx_processed = idx_ch0 + N_SAMPLE_POINTS * (N_BOARD_CHANNELS - 1);
        return false;
    }

    // Judge back-edge time, give whole event time
    int idx_end = 0;
    for (int idx_search = idx_ch0 + N_SAMPLE_POINTS * N_BOARD_CHANNELS - 1; idx_search < (int)(iter_end - iter_first); idx_search++)
    {
        if (*(iter_first + idx_search) < thr_signal)
        {
            idx_end = idx_search;
        }
    }

    // If back-edge not found, than this may be the end of data buffer. May need to copy into another buffer
    if (idx_end == 0)
    {
        idx_processed = (int)(iter_end - iter_first);
        return false;
    }
    idx_processed = idx_end;

    // Judge data length, if it's too long, return false data
    if (idx_processed > N_SAMPLE_POINTS * N_BOARD_CHANNELS + 20)
        return false;
    return true;
}

bool DataManager::ProcessOneTDCEvent(const uint32_t *const iter_first, const uint32_t *const iter_end, int &dataLength)
{
    dataLength = 0;
    if ((int)(iter_end - iter_first) < fADCPointFactor + 2)
        return false;

    if ((*(iter_first + fADCPointFactor) != 65535) || (*(iter_first + fADCPointFactor + 1) != 65535))
    {
        for (auto iter = iter_first + 2; iter != iter_end - 1; iter++)
        {
            if (((*iter) == 65535) && (*(iter + 1) == 65535))
            {
                dataLength = (int)(iter - iter_first);
            }
        }
        // If not find next head, tell call function to break, data is broken
        return false;
    }

    // Process trigger id
    fTDCid = (*(iter_first + 2) << 16) + (*(iter_first + 3) & 0xffff);
    // Process time stamp
    for (int ch = 0; ch < N_BOARD_CHANNELS + 1; ch++)
    {
        auto start_iter = iter_first + 4 + 4 * ch;
        uint64_t temp = *(start_iter);
        fTDCtime[N_BOARD_CHANNELS - ch] = (temp & 0xffff) << 32;
        temp = *(start_iter + 1);
        fTDCtime[N_BOARD_CHANNELS - ch] += (temp & 0xffff) << 16;
        fTDCtime[N_BOARD_CHANNELS - ch] += *(start_iter + 2); // Coarse time
        fTDCtime[N_BOARD_CHANNELS - ch] -= *(start_iter + 3); // fine time
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
    // fTree = NULL;
    fHGTree = NULL;
    fLGTree = NULL;
    fTDCTree = NULL;
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
    // fTree = (TTree *)fFile->Get("fifo");
    fHGTree = (TTree *)fFile->Get("HGTree");
    fLGTree = (TTree *)fFile->Get("LGTree");
    fTDCTree = (TTree *)fFile->Get("TDCTree");

    // fTree->Branch("data", fPreviousData, "data[1600]/i");
    // fTree->SetBranchAddress("ch", fHGamp);
    fHGTree->SetBranchAddress("chHG", fHGamp);
    fLGTree->SetBranchAddress("chLG", fLGamp);
    fTDCTree->SetBranchAddress("chTDC", fTDCtime);
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

    if (!IsOpen() || !fHGTree)
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
    fHGTree->Draw(Form("chHG[%d]>>hHG%d(2047, 2048, 4095)", ch, ch));
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

    if (!IsOpen() || !fLGTree)
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
    fLGTree->Draw(Form("chLG[%d]>>hLG%d(2047, 2048, 4095)", ch, ch));
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
    if (!IsOpen() || !fTDCTree)
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
    fTDCTree->Draw(Form("chTDC[%d]>>hTDC%d(2 ^ 16, 0, 2 ^ 32)", ch, ch));
    hTDC = (TH1F *)fFile->Get(Form("hTDC%d", ch));

    if (!hTDC)
        return false;

    hTDC->SetTitle(Form("TDC Draw channel:%d;ADC Value; Counts", ch));
    return true;
}