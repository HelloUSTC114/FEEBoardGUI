#ifndef ANALYZER_H
#define ANALYZER_H

#include <string>
#include <sstream>
#include <vector>

#define gFTFolderParser (FTFolderParser::Instance())
#define gFTAnalyzer (FTAnalyzer::Instance())

class TDatime;
class TSystemDirectory;

class FiberTest
{
    friend bool operator==(FiberTest &fiber1, FiberTest &fiber2);
    friend bool operator>(FiberTest &fiber1, FiberTest &fiber2);

public:
    FiberTest();
    ~FiberTest();
    void Print();
    bool ProcessFileName(std::string fFileName);

    const std::string &GetName() const { return fFileName; };
    const int GetFiber() const { return fFiber; };
    const int GetEnd() const { return fEnd; };
    const int GetTestSerial() const { return fTestSerial; };
    const TDatime *GetDatime() const { return fDatime; };

private:
    double compareADC[3];
    double testADC[3];
    int fFiber;
    int fEnd;
    int fTestSerial;
    TDatime *fDatime = NULL;
    std::string fFileName;

    static std::stringstream gStringStream;
};

class FTFolderParser
{
public:
    FTFolderParser();
    FTFolderParser(std::string sfolder);
    ~FTFolderParser();
    void Init(std::string sfolder);
    void Print();

    std::string GetFolderName() { return fFolderName; }
    std::vector<FiberTest *> &GetArray() { return fFibers; }

    static FTFolderParser *&Instance();

private:
    void ClearArray();
    TSystemDirectory *dir = NULL;
    std::string fFolderName;
    std::vector<FiberTest *> fFibers;
};

class TF1;
class FTAnalyzer
{
public:
    FTAnalyzer();
    ~FTAnalyzer();
    bool FitFile(std::string sFileName);
    bool FitFile(std::string sFileName, int ch, double *par = NULL, double *parE = NULL);
    const double *GetChPar(int &ch);
    const double *GetChParE(int &ch);
    bool IsChValid() { return fChFlag; }

    bool IsValid() { return fFitFlag; }
    const double *GetPar18() { return par18; };
    const double *GetParE18() { return parE18; };
    const double *GetPar16() { return par16; };
    const double *GetParE16() { return parE16; };

    static FTAnalyzer *&Instance();

private:
    TF1 *fGaus = NULL;

    bool fFitFlag = 0;
    double par18[3];
    double parE18[3];
    double par16[3];
    double parE16[3];

    bool fChFlag = 0;
    double parCh[3], parECh[3];
    int fCh;
};

class Analyzer
{
public:
    Analyzer();
};

#endif // ANALYZER_H
