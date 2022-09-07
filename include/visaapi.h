#ifndef VISAAPI_H
#define VISAAPI_H

// NI-VISA
#ifdef VISAAPI_CXX
#include <visa.h>
#endif

#include <string>

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
    VisaAPI();
    ~VisaAPI();
    int SetAmp(int amp);                         // in unit of mVpp
    int SetHigh(double high);                    // in unit of mV
    int SetLow(double low);                      // in unit of mV
    int SetOffset(double offset);                // in unit of mV
    int SetWaveForm(AFGWaveform wave);           // set waveform
    int SetChannelStatus(int ch, bool openFlag); // Open/close channel output

    //! TODO: add set freq
    int SetFreq(int ch, double freq); // set channel Frequency

    int WriteCMD(std::string sCmd);
    int ReadBuf();

#ifdef VISAAPI_CXX
    int ProcessError(ViStatus status);
    // int ProcessError(ViSession &defaultRM, ViSession &device, ViStatus status, ViChar *buffer);

private:
    void ScanDevice();
    void SetDevice();
    void Clear();
    void InitDevice();

    bool fDeviceFound = 0;
    ViSession defaultRM = VI_NULL, device = VI_NULL;
    ViFindList deviceList = 0;
    ViUInt32 sendCharCount, itemCnt;
    ViChar deviceName[256], buffer[256];

    bool OpenDevice();
    void CloseDevice();
#endif
};

#endif // VISAAPI_H
