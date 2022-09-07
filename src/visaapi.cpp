#define VISAAPI_CXX
#pragma warning(disable : 4996)
#include "visaapi.h"

#include <iostream>

std::string ConvertAFGWaveform(AFGWaveform waveform)
{
    switch (waveform)
    {
    case (Pulse):
        return "Pulse";
    case (DC):
        return "DC";
    case (USER1):
        return "USER1";
    case (USER2):
        return "USER2";
    case (USER3):
        return "USER3";
    default:
        return "";
    }
    return "";
}

std::string ConvertAFGFreqUnit(AFGFreqUnit unit)
{
    switch (unit)
    {
    case (MHz):
        return "MHz";
    case (kHz):
        return "kHz";
    case (Hz):
        return "Hz";
    default:
        return "";
    }
    return "";
}

VisaAPI::VisaAPI()
{
    OpenDevice();
    // InitDevice();
}

VisaAPI::~VisaAPI()
{
    std::cout << "Destructor" << std::endl;
    CloseDevice();
}

void VisaAPI::InitDevice()
{
    if (!fDeviceFound)
        return;
    int status;

    // Send an ID query.
    status = viWrite(device, (ViBuf) "*idn?", 5, &sendCharCount);

    // Clear the buffer and read the response
    memset(buffer, 0, sizeof(buffer));
    status = viRead(device, (ViBuf)buffer, sizeof(buffer), &sendCharCount);
    // Print the response
    std::cout << "id: " << deviceName << ": " << buffer << std::endl;

    // Core write read
    std::string sCmd;

    // Oscilllator setting
    sCmd = "source:roscillator:source internal";
    WriteCMD(sCmd);

    // sCmd = "source1:function:shape edecay";
    sCmd = "source1:function:shape user1";
    WriteCMD(sCmd);

    sCmd = "source1:frequency:mode CW";
    WriteCMD(sCmd);

    sCmd = "source1:frequency:CW 1.5kHz";
    WriteCMD(sCmd);

    sCmd = "source1:voltage:level:immediate:amplitude 50mVpp";
    WriteCMD(sCmd);

    sCmd = "source1:voltage:level:immediate:offset 0mV";
    WriteCMD(sCmd);

    // Output settings
    sCmd = "output1:impedance 50";
    WriteCMD(sCmd);

    sCmd = "output1:polarity normal";
    WriteCMD(sCmd);

    sCmd = "output:trigger:mode trigger";
    WriteCMD(sCmd);

    // Burst mode setting
    sCmd = "source1:burst:mode triggered";
    WriteCMD(sCmd);

    sCmd = "source1:burst:ncycles 1";
    WriteCMD(sCmd);

    sCmd = "source1:burst:state ON";
    WriteCMD(sCmd);

    sCmd = "source1:burst:tdelay 0ns";
    WriteCMD(sCmd);

    //  on
    sCmd = "output1:state 1";
    WriteCMD(sCmd);
}

bool VisaAPI::OpenDevice()
{
    auto status = viOpenDefaultRM(&defaultRM);
    if (!fDeviceFound)
    {
        viFindRsrc(defaultRM, "TCPIP?*INSTR", &deviceList, &itemCnt, deviceName);
        if (itemCnt > 0)
            fDeviceFound = 1;
    }
    status = viOpen(defaultRM, deviceName, VI_NULL, VI_NULL, &device);
    std::cout << "API open device: " << std::endl;
    std::cout << defaultRM << '\t' << deviceList << '\t' << itemCnt << '\t' << deviceName << '\t' << device << std::endl;
    WriteCMD("*idn?");
    ReadBuf();

    if (status == VI_SUCCESS)
        return true;
    else
        return false;
}

int VisaAPI::WriteCMD(std::string sCmd)
{
    if (!fDeviceFound)
        return 1;
    auto status = viWrite(device, (ViBuf)sCmd.c_str(), sCmd.size(), &sendCharCount);
    _sleep(100);
    if (status < 0)
        ProcessError(status);
    return status;
}

int VisaAPI::ReadBuf()
{
    if (!fDeviceFound)
        return 1;
    auto status = viRead(device, (ViBuf)buffer, 256, &sendCharCount);
    _sleep(100);
    std::cout << "Recieve: " << '\t';
    for (int i = 0; i < sendCharCount; i++)
    {
        std::cout << buffer[i];
    }
    if (status < 0)
        ProcessError(status);
    return status;
}

void VisaAPI::CloseDevice()
{

    viClose(device);
    viClose(defaultRM);
    // std::cout << "Closed." << std::endl;
}

#include <string>
int VisaAPI::SetAmp(int amp)
{
    int status = 0;
    std::string sCmd;
    sCmd = "source1:voltage:level:immediate:amplitude " + std::to_string(amp) + "mVpp";
    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "source1:voltage:level:immediate:amplitude?";
    double recieveAmp = 0;
    // char *buf[256];
    status = viQueryf(device, (ViString)sCmd.c_str(), "%lf", &recieveAmp);
    std::cout << "Recieved Amplitude: " << recieveAmp * 1000 << " mV." << std::endl;
    _sleep(100);
    return 0;
}

int VisaAPI::SetHigh(double high)
{
    int status = 0;
    char buf[20];
    sprintf(buf, "%.2f", high);
    std::string sCmd;
    std::string sHigh = buf;

    sCmd = "source1:voltage:high " + sHigh + "mV";
    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "source1:voltage:high?";
    double recieveAmp = 0;
    // char *buf[256];
    status = viQueryf(device, (ViString)sCmd.c_str(), "%lf", &recieveAmp);
    std::cout << "Recieved high: " << recieveAmp * 1000 << " mV." << std::endl;
    _sleep(100);
    return 0;
}

int VisaAPI::SetLow(double low)
{
    int status = 0;
    char buf[20];
    sprintf(buf, "%.2f", low);
    std::string sCmd;
    std::string sLow = buf;

    sCmd = "source1:voltage:low " + sLow + "mV";
    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "source1:voltage:low?";
    double recieveAmp = 0;
    // char *buf[256];
    status = viQueryf(device, (ViString)sCmd.c_str(), "%lf", &recieveAmp);
    std::cout << "Recieved low: " << recieveAmp * 1000 << " mV." << std::endl;
    _sleep(100);
    return 0;
}

int VisaAPI::SetOffset(double offset)
{
    int status = 0;
    char buf[20];
    sprintf(buf, "%.2f", offset);
    std::string sCmd;
    std::string sOffset = buf;

    sCmd = "source1:voltage:offset " + sOffset + "mV";
    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "source1:voltage:offset?";
    double recieveAmp = 0;
    // char *buf[256];
    status = viQueryf(device, (ViString)sCmd.c_str(), "%lf", &recieveAmp);
    std::cout << "Recieved offset: " << recieveAmp * 1000 << " mV." << std::endl;
    _sleep(100);
    return 0;
}

int VisaAPI::SetChannelStatus(int ch, bool openFlag)
{
    if (ch < 1 || ch > 2)
        return 1;
    int status = 0;
    char buf[80];
    sprintf(buf, "output%d: %d", ch, openFlag);
    std::string sCmd = buf;

    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "source1?";
    int recieveAmp = 0;
    // char *buf[256];
    status = viQueryf(device, (ViString)sCmd.c_str(), "%d", &recieveAmp);
    std::cout << "Channel " << ch << " status: " << recieveAmp << std::endl;
    _sleep(100);
    return 0;
}

int VisaAPI::SetWaveForm(AFGWaveform wave)
{
    int status = 0;
    std::string sCmd;
    std::string sWave = ConvertAFGWaveform(wave);
    if (sWave == "")
        return 1;
    sCmd = "source1:function:shape " + sWave + "mVpp";
    int rtn = WriteCMD(sCmd);
    if (rtn < 0)
        return rtn;

    // Read amplitude
    sCmd = "source1:function:shape?";
    // double recieveAmp = 0;
    memset(buffer, '\0', 256 * sizeof(char));
    status = viQueryf(device, (ViString)sCmd.c_str(), "%s", buffer);
    std::cout << "Recieved Amplitude: " << buffer << " mV." << std::endl;
    _sleep(100);
    return 0;
}

int VisaAPI::ProcessError(ViStatus status)
{
    memset(buffer, '\0', 256 * sizeof(char));
    // Report error and clean up
    viStatusDesc(device, status, buffer);
    std::cout << "Failure: " << buffer << std::endl;
    return 1;
}
