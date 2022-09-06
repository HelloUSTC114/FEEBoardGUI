#ifndef VISAAPI_H
#define VISAAPI_H

// NI-VISA
#include <visa.h>
#include <string>

class VisaAPI
{
public:
    VisaAPI();
    ~VisaAPI();
    int SetAmp(int amp); // in unit of mVpp

    // int ProcessError(ViSession &defaultRM, ViSession &device, ViStatus status, ViChar *buffer);
    int ProcessError(ViStatus status);

    int WriteCMD(std::string sCmd);
    int ReadBuf();

private:
    void ScanDevice();
    void SetDevice();
    void Clear();
    void Init();

    void SetFreq(int ch, double freq);
    void SetChannelStatus(int ch, bool openFlag);

    bool fDeviceFound = 0;
    ViSession defaultRM = VI_NULL, device = VI_NULL;
    ViFindList deviceList = 0;
    ViUInt32 sendCharCount, itemCnt;
    ViChar deviceName[256], buffer[256];

    bool OpenDevice();
    void CloseDevice();
};

#endif // VISAAPI_H
