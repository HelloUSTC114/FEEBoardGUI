#include "visaapi.h"

#include <iostream>

VisaAPI::VisaAPI()
{
    OpenDevice();
    Init();
}

VisaAPI::~VisaAPI()
{
    std::cout << "Destructor" << std::endl;
    CloseDevice();
}

void VisaAPI::Init()
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
    int errorCount = 0;
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
    // Open resource found in rsrc deviceList
    status = viOpen(defaultRM, deviceName, VI_NULL, VI_NULL, &device);

    int errorCount = 0;
    std::string sCmd;
    sCmd = "source1:voltage:level:immediate:amplitude " + std::to_string(amp) + "mVpp";
    WriteCMD(sCmd);

    if (status < 0)
    {
        ProcessError(status);
    }

    // Read amplitude
    sCmd = "source1:voltage:level:immediate:amplitude?";
    double recieveAmp = 0;
    // char *buf[256];
    status = viQueryf(device, (ViString)sCmd.c_str(), "%lf", &recieveAmp);
    std::cout << recieveAmp * 1000 << '\t' << (amp == (recieveAmp * 1000)) << std::endl;
    _sleep(1000);
    return 0;
}

int VisaAPI::ProcessError(ViStatus status)
{
    ViChar buffer[256];
    // Report error and clean up
    viStatusDesc(device, status, buffer);
    std::cout << "Failure: " << buffer << std::endl;
    return 1;
}
