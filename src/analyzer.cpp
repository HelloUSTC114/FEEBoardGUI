#include "../include/analyzer.h"

#include <TDatime.h>

#include <iostream>
#include <sstream>
using namespace std;

stringstream FiberTest::gStringStream;

FiberTest::FiberTest()
{
    fDatime = new TDatime;
}

FiberTest::~FiberTest()
{
    /*
    TDatime do not have any variables, only some members, virtual destruct function contains nothing.
    There's no need to delete fDatime.
    If delete fDatime while destructing, the program will crush, according to several test.
    */
    // delete fDatime;
    fDatime = NULL;
}

void FiberTest::Print()
{
    cout << "File: " << fFileName << endl;
    cout << "Fiber: " << fFiber << '\t';
    cout << "End: " << fEnd << '\t';
    cout << "Test Serial: " << fTestSerial << '\t';
    cout << fDatime->AsString() << endl;
}

bool operator==(FiberTest &fiber1, FiberTest &fiber2)
{
    if (fiber1.fFiber != fiber2.fFiber || fiber1.fEnd != fiber2.fEnd)
        return false;
    // fiber1.CompareDate(fiber2);
    return true;
}

bool operator>(FiberTest &fiber1, FiberTest &fiber2)
{
    if (fiber1.fFiber > fiber2.fFiber)
        return true;
    if (fiber1.fFiber < fiber2.fFiber)
        return false;
    if (fiber1.fEnd > fiber2.fEnd)
        return true;
    if (fiber1.fEnd < fiber2.fEnd)
        return false;

    if (*(fiber1.fDatime) > *(fiber2.fDatime))
    {
        fiber1.fTestSerial = fiber2.fTestSerial + 1;
        return true;
    }
    else if (*(fiber1.fDatime) < *(fiber2.fDatime))
    {
        fiber2.fTestSerial = fiber1.fTestSerial + 1;
        return false;
    }
    return false;
}

bool FiberTest::ProcessFileName(string filename)
{
    int serialIndex = 0, endIndex = 0;
    serialIndex = (int)filename.find("Fiber");
    endIndex = (int)filename.find("End");

    if (serialIndex == string::npos || endIndex == string::npos)
        return false;

    fFileName = filename;
    string sSerial = filename.substr(serialIndex + 5, endIndex - 1 - (serialIndex + 5));
    string sEnd = filename.substr(endIndex + 3);
    gStringStream.clear();
    gStringStream.str(sEnd);
    // string s;
    char c;
    int serial = stoi(sSerial);
    int end;
    gStringStream >> end >> c;

    fFiber = serial;
    fEnd = end;
    fTestSerial = 0;

    int yy, MM, dd, hh, mm, ss;
    gStringStream >> yy >> c >> MM >> c >> dd >> c >> hh >> c >> mm >> c >> ss;
    fDatime->Set(yy, MM, dd, hh, mm, ss);
    // Print();

    return true;
}

#include <TSystemDirectory.h>
FTFolderParser::FTFolderParser()
{
    dir = new TSystemDirectory("dir", "./");
}
FTFolderParser::FTFolderParser(string sFolder)
{
    dir = new TSystemDirectory("dir", "./");
    Init(sFolder);
}

FTFolderParser::~FTFolderParser()
{
    ClearArray();
    delete dir;
    dir = NULL;
}

FTFolderParser *&FTFolderParser::Instance()
{
    static FTFolderParser *instance = new FTFolderParser;
    return instance;
}

#include <TList.h>
void FTFolderParser::Init(string sfolder)
{
    if (fFibers.size() > 0)
        ClearArray();
    dir->SetDirectory(sfolder.c_str());
    fFolderName = sfolder;

    auto files = dir->GetListOfFiles();
    if (!files)
        return;
    for (int i = 0; i < files->GetEntries(); i++)
    {
        string fname = files->At(i)->GetName();
        if (fname.find(".root") == string::npos || fname.find("Fiber") == string::npos || fname.find("End") == string::npos)
            continue;

        auto fiber = new FiberTest;
        fiber->ProcessFileName(fname);
        fFibers.push_back(fiber);
    }

    sort(fFibers.begin(), fFibers.end(), [](FiberTest *fiber1, FiberTest *fiber2) -> bool
         { return !((*fiber1) > (*fiber2)); });
}

void FTFolderParser::ClearArray()
{
    for (int i = 0; i < fFibers.size(); i++)
    {
        delete fFibers[i];
        fFibers[i] = NULL;
    }
    fFibers.clear();
}

void FTFolderParser::Print()
{
    cout << "Folder Name: " << fFolderName << endl;
    for (int i = 0; i < fFibers.size(); i++)
    {
        fFibers[i]->Print();
    }
}

#include <TF1.h>
FTAnalyzer::FTAnalyzer()
{
    fGaus = new TF1("fGausFTAnalyzer", "gaus", 0, 4096);
}

FTAnalyzer::~FTAnalyzer()
{
    delete fGaus;
    fGaus = NULL;
}

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
bool FTAnalyzer::FitFile(std::string sFileName)
{
    auto file = new TFile(sFileName.c_str());
    auto tree = (TTree *)file->Get("fifo");
    if (!tree)
        return false;

    tree->Draw("ch[16]>>h1(2047, 2048, 4095)");
    auto h1 = (TH1F *)(gFile->Get("h1"));
    h1->Fit(fGaus, "RQ");
    for (int i = 0; i < 3; i++)
    {
        par16[i] = fGaus->GetParameter(i);
        parE16[i] = fGaus->GetParError(i);
    }

    tree->Draw("ch[18]>>h2(2047, 2048, 4095)");
    auto h2 = (TH1F *)(gFile->Get("h2"));
    h2->Fit(fGaus, "RQ");
    for (int i = 0; i < 3; i++)
    {
        par18[i] = fGaus->GetParameter(i);
        parE18[i] = fGaus->GetParError(i);
    }

    fFitFlag = 1;
    file->Close();
    delete file;

    return true;
}

bool FTAnalyzer::FitFile(std::string sFileName, int ch, double *par, double *parE)
{
    auto file = new TFile(sFileName.c_str());
    auto tree = (TTree *)file->Get("fifo");
    if (!tree)
        return false;

    tree->Draw(Form("ch[%d]>>h(2047, 2048, 4095)", ch));
    auto h = (TH1F *)(gFile->Get("h"));
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
    file->Close();
    delete file;
    return true;
}

const double *FTAnalyzer::GetChPar(int &ch)
{
    ch = fCh;
    return parCh;
}

const double *FTAnalyzer::GetChParE(int &ch)
{
    ch = fCh;
    return parECh;
}

FTAnalyzer *&FTAnalyzer::Instance()
{
    static FTAnalyzer *instance = new FTAnalyzer;
    return instance;
}

Analyzer::Analyzer()
{
}
