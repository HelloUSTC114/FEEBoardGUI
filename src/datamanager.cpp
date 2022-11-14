#include "../include/datamanager.h"

#include "feecontrol.h"
#include <TTree.h>
#include <TFile.h>
#include <TH1S.h>
#include <TH1I.h>
#include <TMath.h>
#include <iostream>

using namespace std;

#define CONVERT_ADC_2_AMP(adc) ((double)adc / 2.048 - 1000)
#define CONVERT_AMP_2_ADC(amp) (2.048 * amp + 2048)

double gEventADCThreshold = CONVERT_AMP_2_ADC(100);
double DataManager::fgFreq = 433.995; // in unit of MHz, board TDC frequency
int DataManager::fADCPointFactor = FEEControl::fHGPointFactor;
int DataManager::fTDCPointFactor = FEEControl::fTDCPointFactor;

double DataManager::ConvertTDC2Time(uint64_t tdc, double &coarseTime, double &fineTime)
{
    coarseTime = (tdc >> 16) / fgFreq * 1e3;
    fineTime = (tdc & 0xffff) / 65536.0 / fgFreq * 1e3;
    return (coarseTime - fineTime);
}

DataManager *gDataManager = new DataManager();

DataManager::DataManager()
{
    fPreviousData = new uint32_t[N_BOARD_CHANNELS * N_SAMPLE_POINTS];
    memset(fPreviousData, '\0', sizeof(uint32_t) * N_BOARD_CHANNELS * N_SAMPLE_POINTS);

    fHGBuffer = new uint16_t[(int)(2.5 * fADCPointFactor / 2)];
    fLGBuffer = new uint16_t[(int)(2.5 * fADCPointFactor / 2)];
    fTDCBuffer = new uint16_t[(int)(2.5 * fTDCPointFactor / 2)];
    memset(fHGBuffer, '\0', sizeof(uint16_t) * (int)(2.5 * fADCPointFactor / 2));
    memset(fLGBuffer, '\0', sizeof(uint16_t) * (int)(2.5 * fADCPointFactor / 2));
    memset(fTDCBuffer, '\0', sizeof(uint16_t) * (int)(2.5 * fTDCPointFactor / 2));
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
        fHGHist[i] = new TH1S(Form("hHG%d", i), Form("High Gain ch%d", i), 4096, 0, 65536);
        fLGHist[i] = new TH1S(Form("hLG%d", i), Form("Low Gain ch%d", i), 4096, 0, 65536);
        fTDCHist[i] = new TH1I(Form("hTDC%d", i), Form("TDC Value ch%d", i), 2 ^ 16, 0, 2 ^ 32);
    }
    fTDCHist[32] = new TH1I(Form("hTDC%d", 32), Form("TDC Value ch%d", 32), 2 ^ 16, 0, 2 ^ 32);

    // fTree->Branch("data", fPreviousData, "data[1600]/i");
    fHGTree->Branch("chHG", fHGamp, "chHG[32]/D");
    fHGTree->Branch("chHGid", &fHGid, "chHGid/i");
    fLGTree->Branch("chLG", fLGamp, "chLG[32]/D");
    fLGTree->Branch("chLGid", &fLGid, "chLGid/i");
    fTDCTree->Branch("chTDCid", &fTDCid, "chTDCid/i");
    fTDCTree->Branch("chTDC", fTDCTime, "chTDC[33]/l");

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

bool DataManager::Draw(int ch, DrawOption option)
{
    if (option == HGDataDraw)
        return DrawHG(ch);
    else if (option == LGDataDraw)
        return DrawLG(ch);
    else if (option == TDCDataDraw)
        return DrawTDC(ch);
    else
        return false;
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

void DataManager::ClearDraw()
{
    if (!IsOpen() || !fHGHist[0])
        return;
    for (int ch = 0; ch < 32; ch++)
    {
        fHGHist[ch]->Reset("ICESM");
        fLGHist[ch]->Reset("ICESM");
        fTDCHist[ch]->Reset("ICESM");
    }
    fTDCHist[32]->Reset("ICESM");
}

int DataManager::ProcessFEEData(FEEControl *fee)
{
    // Previous processing codes
    // if (fee == NULL || !fee->GetDataValidation())
    // {
    //     return 0;
    // }

    // int eventCounter = 0;
    // int currentIndex = 0;
    // while (ProcessOneEvent(fee, currentIndex))
    // {
    //     eventCounter++;
    // }

    // return eventCounter;

    if (fee == NULL || !fee->GetDataValidation())
    {
        return 0;
    }
    //! TODO: add board change validation;
    // bool flagBoardChange

    fFEECurrentProcessing = fee;
    fHGLastLoopEventCount = ProcessADCEvents(0);
    fLGLastLoopEventCount = ProcessADCEvents(1);
    fTDCLastLoopEventCount = ProcessTDCEvents();

    return fHGLastLoopEventCount;
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

void DataManager::FillTDCData()
{
    fTDCTree->Fill();
    for (int ch = 0; ch < 33; ch++)
    {
        // fTDCHist[ch]->Fill(fTDCTime[ch] - fTDCFineTime[ch] / 65536.0);
        fTDCHist[ch]->Fill(ConvertTDC2Time(fTDCTime[ch]));
    }
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

void DataManager::PrintHGBuffer()
{
    std::cout << "Only for debug: Print HG buffer inside DataManager:" << std::endl;
    std::cout << "Buffer Count: " << fHGBufCount << std::endl;
    for (int i = 0; i < fHGBufCount; i++)
    {
        std::cout << fHGBuffer[i];
        if (i % 20 != 20 - 1)
            std::cout << '\t';
        else
            std::cout << endl;
    }
    std::cout << "HG buffer inside DataManager print done." << std::endl;
}

void DataManager::PrintLGBuffer()
{
    std::cout << "Only for debug: Print LG buffer inside DataManager:" << std::endl;
    std::cout << "Buffer Count: " << fLGBufCount << std::endl;
    for (int i = 0; i < fLGBufCount; i++)
    {
        std::cout << fLGBuffer[i];
        if (i % 20 != 20 - 1)
            std::cout << '\t';
        else
            std::cout << endl;
    }
    std::cout << "LG buffer inside DataManager print done." << std::endl;
}

void DataManager::PrintTDCBuffer()
{
    std::cout << "Only for debug: Print TDC buffer inside DataManager:" << std::endl;
    std::cout << "Buffer Count: " << fTDCBufCount << std::endl;
    for (int i = 0; i < fTDCBufCount; i++)
    {
        std::cout << fTDCBuffer[i];
        if (i % 20 != 20 - 1)
            std::cout << '\t';
        else
            std::cout << endl;
    }
    std::cout << "TDC buffer inside DataManager print done." << std::endl;
}

int DataManager::ProcessADCEvents(int adcNo)
{
    if (adcNo != 0 && adcNo != 1)
        return -1;

    // const uint32_t *adcdata = NULL;
    const uint16_t *adcdata = NULL;
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
    return ProcessADCEvents(adcNo, adcdata, adcdatalength);
}

int DataManager::ProcessADCEvents(int adcNo, const uint16_t *src_data, int dataLength)
{
    if (adcNo != 0 && adcNo != 1)
        return -1;

    const uint16_t *adcdata = src_data;
    int adcdatalength = dataLength;

    int headFirst = 0, headLast = 0, totalEventCounter = 0;
    // std::cout << "Test: ----------------------" << std::endl;

    // if there're points left in buffer, concatenate this new data with buffer inside
    if (fADCBufCount[adcNo] > 0)
    {
        // std::cout << "Test: Processing Buffer: " << std::endl;
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
        memcpy(fADCBuffer[adcNo] + fADCBufCount[adcNo], adcdata, headFirst * sizeof(uint16_t));
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
        else
        {
            std::cout << "John Test: Error while Processing ADC: error inside Manager buffer" << std::endl;
        }

        // Judge whether contains 2 heads
        int headInside = 0;
        for (int idx_search = idx_processed > 2 ? idx_processed : 2; idx_search < fADCBufCount[adcNo] - 1; idx_search++)
        {
            if (fADCBuffer[adcNo][idx_search] == 65535 && fADCBuffer[adcNo][idx_search + 1] == 65535)
            {
                // std::cout << idx_processed << '\t' << std::endl;
                // std::cout << "Test: searching Head inside buffer. " << headInside << '\t' << fADCBuffer[adcNo][idx_search] << '\t' << fADCBuffer[adcNo][idx_search + 1] << std::endl;
                // std::cout << "Test: Buffer Length: " << fADCBufCount[0] << std::endl;
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
                std::cout << "Warning: double data in ADC buffer " << adcNo << std::endl;
                // PrintHGBuffer();
                // Fill Data
                FillADCData(adcNo);
                totalEventCounter++;
                fADCEventCount[adcNo]++;
            }
            else
            {
                std::cout << "John Test: Error while Processing ADC: error for 2nd data inside Manager buffer" << std::endl;
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
        // std::cout << "Test: Head found in source: " << head << std::endl;

        // if this event is the last, copy memory to buffer, and record buffer length, then break this process loop
        if (head + fADCPointFactor > adcdatalength - 10)
        {
            fADCBufCount[adcNo] = adcdatalength - head;
            memcpy(fADCBuffer[adcNo], adcdata + head, (adcdatalength - head) * sizeof(uint16_t));
            break;
        }

        int idx_processed = 0;
        bool dataflag = ProcessOneADCEvent(adcdata + head, adcdata + adcdatalength, fADCid[adcNo], fADCamp[adcNo], idx_processed);

        if (!dataflag)
        {
            std::cout << "John Test: Error while Processing ADC: error in normal Processing" << std::endl;

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

        idx_search += idx_processed - 3; // skip data points, so as to save plenty of searching time
    }

    return totalEventCounter;
}

#include <fstream>
std::ofstream fout("test2.dat");

bool DataManager::ProcessOneADCEvent(const uint16_t *const iter_first, const uint16_t *const iter_end, uint32_t &id, double *chArray, int &idx_processed)
{
    if ((iter_end - iter_first) < 175)
    {
        idx_processed = 0;
        return false;
    }

    // Judge Head
    if (*(iter_first) != 65535 || *(iter_first + 1) != 65535)
    {
        idx_processed = 2;
        return false;
    }
    // Process trigger id
    id = (*(iter_first + 2) << 16) + (*(iter_first + 3) & 0xffff);

    // Judge whether data length
    for (int i = 4; i < 10; i++)
    {
        if (*(iter_first + i) == 65535)
        {
            idx_processed = i;
            return false;
        }
    }

    // Find Start data
    int idx_start = 7;

    // Drop the 3rd data, for its deviation is large
    for (int ch = 0; ch < N_BOARD_CHANNELS; ch++)
    {
        chArray[ch] = *(iter_first + idx_start + 5 * ch) + *(iter_first + idx_start + 5 * ch + 1) + *(iter_first + idx_start + 5 * ch + 3) + *(iter_first + idx_start + 5 * ch + 4);
        chArray[ch] /= 4;
    }
    idx_processed = idx_start + 5 * N_BOARD_CHANNELS;
    return true;
}

/*
// Previous ADC Events Processor
bool DataManager::ProcessOneADCEvent(const uint16_t *const iter_first, const uint16_t *const iter_end, uint32_t &id, double *chArray, int &idx_processed)
{
    // Process trigger id
    id = (*(iter_first + 2) << 16) + (*(iter_first + 3) & 0xffff);
    // std::cout << "Test: Processing ADC: " << std::endl;

    // thrshold Calculating
    double ped = TMath::Mean(iter_first + 5, iter_first + 25);
    double pedrms = TMath::StdDev(iter_first + 5, iter_first + 25);
    double thr_signal_front = ped + pedrms * 10;    // set front threshold at pedestal + 10 sigma
    double thr_signal_end = thr_signal_front + 512; //
    // std::cout << pedrms << std::endl;
    // std::cout << "Test: Threshold: " << thr_signal_front << std::endl;

    // Find start data
    int idx_ch0 = 0;
    for (auto idx_search = 25; idx_search < (int)(iter_end - iter_first); idx_search++)
    {
        if (*(iter_first + idx_search) > thr_signal_front)
        {
            idx_ch0 = idx_search;
            break;
        }
    }
    if (idx_ch0 == 0)
    {
        std::cout << "Error in ADC Process single event: Cannot find signal start point: threshold: " << thr_signal_front << std::endl;

        idx_processed = 0;
        return false;
    }

    // std::cout << "Test: find idx_ch0: " << idx_ch0 << '\t' << *(iter_first + idx_ch0) << std::endl;
    // id = (*);
    // id = (hgdata[head + 2] << 16) + (hgdata[head + 3] & 0xffff);
    for (int ch = 0; ch < N_BOARD_CHANNELS; ch++)
    {
        auto iterFirst = iter_first + idx_ch0 + N_SAMPLE_POINTS * ch;
        auto iterEnd = iter_first + idx_ch0 + N_SAMPLE_POINTS * ch + N_SAMPLE_POINTS - 1;
        int boardLength = 5; // abort edge points
        chArray[ch] = TMath::Mean(iterFirst + boardLength, iterEnd - boardLength);
        // std::cout << "Test Channel Mean: " << ch << '\t' << chArray[ch] << std::endl;

        // data error processing, if one channel is beyond pedestal, it may reveals data error, and this will be a invalid data.
        if (chArray[ch] < thr_signal_end)
        {
            std::cout << "Error in ADC Process single event: Invalid channel: " << ch << ",  below threshold may reveals invalid data." << std::endl;
            idx_processed = idx_ch0 + N_SAMPLE_POINTS * (ch - 1 > 0 ? ch - 1 : 0);
            return false;
        }
    }
    // In case of all 31 channels are beyond threshold, but the last few points were set into below pedestal
    if (*(iter_first + idx_ch0 + N_SAMPLE_POINTS * N_BOARD_CHANNELS - 15) < thr_signal_end)
    {
        std::cout << "Error in ADC Process single event: Invalid last points,  below threshold may reveals invalid data" << std::endl;
        idx_processed = idx_ch0 + N_SAMPLE_POINTS * (N_BOARD_CHANNELS - 1);
        return false;
    }

    // Judge back-edge time, give whole event time
    int idx_end = 0;
    for (int idx_search = idx_ch0 + N_SAMPLE_POINTS * N_BOARD_CHANNELS - 1; idx_search < (int)(iter_end - iter_first); idx_search++)
    {
        if (*(iter_first + idx_search) < thr_signal_end)
        {
            // std::cout << "Test: back-edge searching: " << idx_search << '\t' << *(iter_first + idx_search) << std::endl;
            idx_end = idx_search;
            break;
        }
    }

    // If back-edge not found, than this may be the end of data buffer. May need to copy into another buffer
    if (idx_end == 0)
    {
        idx_processed = (int)(iter_end - iter_first);
        std::cout << "Error in ADC Process single event: Not found back-edge in this signal." << std::endl;
        return false;
    }
    idx_processed = idx_end;

    // std::cout << "Test: Process done, idx_processed: " << idx_processed << std::endl;
    // Judge data length, if it's too long, return false data
    if (idx_processed > N_SAMPLE_POINTS * N_BOARD_CHANNELS + idx_ch0 + 20)
    {
        // static int countTest = 0;
        // std::cout << "Error in ADC Process single event: data back-edge is too long: Error Count: " << countTest++ << '\t' << idx_end - idx_ch0 << '\t' << idx_ch0 << '\t' << idx_end << '\t' << N_SAMPLE_POINTS * N_BOARD_CHANNELS << std::endl;
        // std::cout << "Threshold: " << thr_signal_front << std::endl;
        // for (int i = 0; i < idx_ch0 + 10; i++)
        // {
        //     std::cout << *(iter_first + i);
        //     if (i % 30 == 30 - 1)
        //         std::cout << std::endl;
        //     else
        //         std::cout << ' ';
        // }
        // std::cout << endl;
        // for (int i = idx_ch0 - 10 + N_SAMPLE_POINTS * N_BOARD_CHANNELS; i < (idx_end +5); i++)
        // {
        //     std::cout << *(iter_first + i);
        //     if (i % 30 == 30 - 1)
        //         std::cout << std::endl;
        //     else
        //         std::cout << ' ';
        // }
        // std::cout << "End of this error output" << std::endl;
        // for (int i = 0; i < (idx_end+5); i++)
        // {
        //     fout << *(iter_first + i) << std::endl;
        //     // if (i % 30 == 30 - 1)
        //     //     fout << std::endl;
        //     // else
        //     //     fout << ' ';
        // }
        // fout << std::endl
        //      << std::endl;
        std::cout << "Error in ADC Process single event: data back-edge is too long." << std::endl;
        return false;
    }
    return true;
}
*/

int DataManager::ProcessTDCEvents()
{
    const uint16_t *tdcdata = fFEECurrentProcessing->GetTDCFIFOData();
    int tdcdatalength = fFEECurrentProcessing->GetTDCDataLength();
    return ProcessTDCEvents(tdcdata, tdcdatalength);
}

int DataManager::ProcessTDCEvents(const uint16_t *src_data, int dataLength)
{
    const uint16_t *tdcdata = src_data;
    int tdcdatalength = dataLength;

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
        memcpy(fTDCBuffer + fTDCBufCount, tdcdata, headFirst * sizeof(uint16_t));
        fTDCBufCount += headFirst;

        int idx_processed = 0;
        bool dataflag = ProcessOneTDCEvent(fTDCBuffer, fTDCBuffer + fTDCBufCount, idx_processed);
        if (dataflag)
        {
            // Fill Data
            // std::cout << "Test: Fill Data in buffer: " << fTDCEventCount << std::endl;
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
        // std::cout << "Test: found Head in source: " << head << '\t' << tdcdata[idx_search] << std::endl;

        // if this event is the last, copy memory to buffer, and record buffer length, then break this process loop
        if (head + fTDCPointFactor > tdcdatalength)
        {
            fTDCBufCount = tdcdatalength - head;
            memcpy(fTDCBuffer, tdcdata + head, (tdcdatalength - head) * sizeof(uint16_t));
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
        // std::cout << "Test: Fill Data: " << fTDCEventCount << std::endl;
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

bool DataManager::ProcessOneTDCEvent(const uint16_t *const iter_first, const uint16_t *const iter_end, int &dataLength)
{
    // std::cout << "Test: Processing a tdc data: " << std::endl;
    dataLength = 0;
    int inputLength = (int)(iter_end - iter_first);

    // Process data in buffer requires inputlength == fTDCPointFactor, else set it as failure
    if (inputLength < fTDCPointFactor || inputLength == fTDCPointFactor + 1)
    {
        std::cout << "Error in processign one data: input length is too short." << inputLength << std::endl;
        return false;
    }

    bool flagBufferData = (inputLength == fTDCPointFactor); // Judge whether is processing buffer data
    if (!flagBufferData)
    {
        bool flagNextHeadValid = (*(iter_first + fTDCPointFactor) != 65535) || (*(iter_first + fTDCPointFactor + 1) != 65535);
        if (flagNextHeadValid)
        {
            for (auto iter = iter_first + 2; iter != iter_end - 1; iter++)
                if (((*iter) == 65535) && (*(iter + 1) == 65535))
                    dataLength = (int)(iter - iter_first);
            // If not find next head, tell call function to break, data is broken
            std::cout << "Error in processign one data: next header does not match 136 constrains. DataLength: " << inputLength << std::endl;
            return false;
        }
    }

    // Process trigger id
    // std::cout << "Test: Successfully varified this tdc data" << std::endl;
    fTDCid = (*(iter_first + 2) << 16) + (*(iter_first + 3) & 0xffff);
    // std::cout << "Test: TDC TriggerID: " << fTDCid << std::endl;
    // Process time stamp
    for (int ch = 0; ch < N_BOARD_CHANNELS + 1; ch++)
    {
        auto start_iter = iter_first + 4 + 4 * ch;
        fTDCTime[N_BOARD_CHANNELS - ch] = 0;
        for (int i = 0; i < 4; i++)
        {
            uint64_t temp = (*(start_iter + i)) & 0xffff;
            fTDCTime[N_BOARD_CHANNELS - ch] += (temp << ((4 - (i + 1)) * 16));
        }
    }
    dataLength = fTDCPointFactor;
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
    fTDCTree->SetBranchAddress("chTDC", fTDCTime);
    // fTDCTree->SetBranchAddress("chTDC", fTDCFineTime);
    return true;
}

ReadManager *&ReadManager::Instance()
{
    static ReadManager *fInstance = new ReadManager();
    return fInstance;
}

bool ReadManager::Draw(int ch, DrawOption option)
{
    // std::cout << "Test: Draw option: " << option << std::endl;
    if (option == HGDataDraw)
        return DrawHG(ch);
    else if (option == LGDataDraw)
        return DrawLG(ch);
    else if (option == TDCDataDraw)
        return DrawTDC(ch);
    else
        return false;
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
    //    std::cout << "Test: Draw HG: " << std::endl;

    auto hHG = (TH1F *)fFile->Get(Form("hHG%d", ch));
    if (hHG)
    {
        hHG->Draw();
        return true;
    }
    fFile->cd();
    fHGTree->Draw(Form("chHG[%d]>>hHG%d(4096, 0, 65536)", ch, ch));
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
    fLGTree->Draw(Form("chLG[%d]>>hLG%d(4096, 0, 65536)", ch, ch));
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

    hTDC->SetTitle(Form("TDC Draw channel:%d;TDC Value; Counts", ch));
    return true;
}
