#ifndef VISAAPI_H
#define VISAAPI_H

// C++
#include <string>
#include <sstream>
#include <vector>

// NI-VISA
#ifdef VISAAPI_CXX
#include <visa.h>
#endif

enum AFGWaveform
{
    Pulse = 0,
    DC,
    USER1,
    USER2,
    USER3,
};
std::string ConvertAFGWaveform(AFGWaveform wave);

enum AFGFreqUnit
{
    MHz = 0,
    kHz,
    Hz,
};
std::string ConvertAFGFreqUnit(AFGFreqUnit unit);

class VisaAPI
{
public:
    VisaAPI() = default;
    VisaAPI(std::string sDeviceName);
    ~VisaAPI();

    bool TryConnectDevice(std::string sDeviceName);
    bool IsConnected() { return fDeviceOpenFlag; };

#ifdef VISAAPI_CXX
    int ProcessError(ViStatus status);
    // int ProcessError(ViSession &defaultRM, ViSession &device, ViStatus status, ViChar *buffer);
    int WriteCMD(std::string sCmd);
    int WriteCMDSerial(std::vector<std::string> vCMDs);
    std::string ReadBuf();
    std::string Query(std::string sCmd);

private:
    void ScanDevice();
    void SetDevice();
    void Clear();
    void InitDevice();

    bool fDeviceFound = 0;
    ViSession defaultRM = VI_NULL, device = VI_NULL;
    ViFindList deviceList = 0;
    ViUInt32 sendCharCount, itemCnt;
    ViChar deviceName[256], buffer[4096], errBuffer[256];

protected:
    bool OpenDevice(std::string sDeviceName);
    void CloseDevice();
    static std::stringstream gss;
#endif
private:
    bool fDeviceOpenFlag = 0;
};

#define gAFGVisa (AFGVisaAPI::Instance())
class AFGVisaAPI : public VisaAPI
{
public:
    AFGVisaAPI();
    static AFGVisaAPI *Instance();
    int SetAmp(int amp);                         // in unit of mVpp
    int SetHigh(double high);                    // in unit of mV
    int SetLow(double low);                      // in unit of mV
    int SetOffset(double offset);                // in unit of mV
    int SetWaveForm(AFGWaveform wave);           // set waveform
    int SetChannelStatus(int ch, bool openFlag); // Open/close channel output

    //! TODO: add set freq
    int SetFreq(int ch, double freq); // set channel Frequency

private:
    std::string fDeviceName;
};

#define gAgi1344Visa (Agi344VisaAPI::Instance())
class Agi344VisaAPI : public VisaAPI
{
public:
    Agi344VisaAPI();
    static Agi344VisaAPI *Instance();
    int SetImpedance(bool autoON);

    int SetSamplePoints(int points = 50);
    int GetSamplePoints(int &points);

    int GetData(std::vector<double> &vData);

    int InitMeasure();
    double MeasureOnce();

    void Clear34410();
    void InitMeasure34410();
    void Measure34410(std::vector<double> &vResult);

private:
    std::string fDeviceName;
    int fNPoints = 50;
};

#endif // VISAAPI_H
