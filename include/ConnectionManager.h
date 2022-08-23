#ifndef ConnectionManager_H
#define ConnectionManager_H

#include <zaber/motion/binary.h>
#include <atlbase.h>
#include <iostream>

#define gconm (ConnectionManager::Instance())

namespace UserDefine
{
    extern std::string NO_DEVICE_FOUND;
}

class ConnectionManager
{
public:
    ~ConnectionManager();
    static ConnectionManager *&Instance();
    inline std::string getPortNameNow() { return sPortNameNow; }
    std::vector<zaber::motion::binary::Device> getDeviceList();

    std::string ScanDevice();
    bool ScanPort(std::string portName);

    bool IsOpen() { return fIsOpen; }

private:
    ConnectionManager();
    inline void ProcessClose()
    {
        sPortNameNow = UserDefine::NO_DEVICE_FOUND;
        fDeviceList.clear();
        fIsOpen = false;
    };
    void Init();

    std::string sPortNameNow;
    zaber::motion::binary::Connection fcon;
    std::vector<zaber::motion::binary::Device> fDeviceList;

    bool fIsOpen;
};

std::vector<std::string> getComPort();

bool TryFindDevice(zaber::motion::binary::Connection &con, std::string portName) noexcept;
bool TryFindDevice(zaber::motion::binary::Connection &con, std::string portName, std::vector<zaber::motion::binary::Device> &deviceList) noexcept;
// return current port name
std::string TryFindDevice(zaber::motion::binary::Connection &con) noexcept;

#endif // ConnectionManager_H
